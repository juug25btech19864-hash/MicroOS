/*
 * MicroOS Arcade - Tetris
 * Classic falling blocks terminal game
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <signal.h>
#include <sys/select.h>

#define BOARD_W 10
#define BOARD_H 20

static struct termios orig_termios;
static int raw_mode_active = 0;

static void disable_raw_mode(void) {
    if (raw_mode_active) {
        printf("\033[?25h\033[0m\n");
        fflush(stdout);
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        raw_mode_active = 0;
    }
}

static void sig_handler(int sig) {
    (void)sig;
    disable_raw_mode();
    exit(0);
}

static void enable_raw_mode(void) {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) return;
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != -1) {
        raw_mode_active = 1;
        atexit(disable_raw_mode);
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);
        printf("\033[?25l");
        fflush(stdout);
    }
}

static int kbhit(void) {
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}

static int read_key(void) {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return 0;
    if (c == '\033') {
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\033';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\033';
        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return 'w'; // Up / Rotate
                case 'B': return 's'; // Down / Soft Drop
                case 'C': return 'd'; // Right
                case 'D': return 'a'; // Left
            }
        }
        return '\033';
    }
    return c;
}

// 7 Tetromino shapes (4x4 matrix representation)
static const int TETROMINOES[7][4][4] = {
    // I
    { {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0} },
    // J
    { {1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} },
    // L
    { {0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} },
    // O
    { {0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} },
    // S
    { {0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0} },
    // T
    { {0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} },
    // Z
    { {1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0} }
};

static const char *TETRO_COLORS[8] = {
    "\033[0m",       // 0: empty
    "\033[1;36m",   // 1: Cyan (I)
    "\033[1;34m",   // 2: Blue (J)
    "\033[1;37m",   // 3: White/Orange (L)
    "\033[1;33m",   // 4: Yellow (O)
    "\033[1;32m",   // 5: Green (S)
    "\033[1;35m",   // 6: Magenta (T)
    "\033[1;31m"    // 7: Red (Z)
};

static int board[BOARD_H][BOARD_W];
static int current_piece[4][4];
static int cur_x = 3, cur_y = 0;
static int cur_type = 0;

static void rotate_piece(void) {
    int temp[4][4];
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            temp[c][3 - r] = current_piece[r][c];
        }
    }
    // Check if valid
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (temp[r][c]) {
                int bx = cur_x + c;
                int by = cur_y + r;
                if (bx < 0 || bx >= BOARD_W || by >= BOARD_H) return;
                if (by >= 0 && board[by][bx]) return;
            }
        }
    }
    memcpy(current_piece, temp, sizeof(temp));
}

static int check_collision(int ox, int oy) {
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (current_piece[r][c]) {
                int bx = ox + c;
                int by = oy + r;
                if (bx < 0 || bx >= BOARD_W || by >= BOARD_H) return 1;
                if (by >= 0 && board[by][bx]) return 1;
            }
        }
    }
    return 0;
}

static void spawn_piece(void) {
    cur_type = rand() % 7;
    memcpy(current_piece, TETROMINOES[cur_type], sizeof(current_piece));
    cur_x = 3;
    cur_y = 0;
}

static void lock_piece(void) {
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (current_piece[r][c]) {
                int bx = cur_x + c;
                int by = cur_y + r;
                if (by >= 0 && by < BOARD_H && bx >= 0 && bx < BOARD_W) {
                    board[by][bx] = cur_type + 1;
                }
            }
        }
    }
}

static int clear_lines(void) {
    int lines_cleared = 0;
    for (int r = BOARD_H - 1; r >= 0; --r) {
        int full = 1;
        for (int c = 0; c < BOARD_W; ++c) {
            if (board[r][c] == 0) { full = 0; break; }
        }
        if (full) {
            lines_cleared++;
            for (int k = r; k > 0; --k) {
                for (int c = 0; c < BOARD_W; ++c) {
                    board[k][c] = board[k - 1][c];
                }
            }
            for (int c = 0; c < BOARD_W; ++c) board[0][c] = 0;
            r++; // Recheck same row
        }
    }
    return lines_cleared;
}

int main(void) {
    enable_raw_mode();
    srand((unsigned int)time(NULL));

restart_tetris:;
    memset(board, 0, sizeof(board));
    int score = 0;
    int lines = 0;
    long delay_ms = 400;
    spawn_piece();

    while (1) {
        // Render
        printf("\033[H\033[2J");
        printf("\033[1;36m=== MICRO ARCADE: TETRIS ===\033[0m\n");
        printf("Score: \033[1;32m%d\033[0m | Lines: \033[1;33m%d\033[0m | Level: %d\n\n", 
               score, lines, (lines / 10) + 1);

        for (int r = 0; r < BOARD_H; ++r) {
            printf("|");
            for (int c = 0; c < BOARD_W; ++c) {
                int is_cur = 0;
                int pr = r - cur_y;
                int pc = c - cur_x;
                if (pr >= 0 && pr < 4 && pc >= 0 && pc < 4 && current_piece[pr][pc]) {
                    is_cur = cur_type + 1;
                }
                int val = is_cur ? is_cur : board[r][c];
                if (val > 0) {
                    printf("%s[]\033[0m", TETRO_COLORS[val]);
                } else {
                    printf(" .");
                }
            }
            printf("|\n");
        }
        printf("+--------------------+\n");
        printf("Controls: [A/D] Move | [W] Rotate | [S] Soft Drop | [Space] Hard Drop | [Q] Quit\n");
        fflush(stdout);

        // Input & timer
        int steps = delay_ms / 20;
        for (int i = 0; i < steps; ++i) {
            if (kbhit()) {
                int k = read_key();
                if (k == 'q' || k == 'Q') {
                    disable_raw_mode();
                    return 0;
                } else if (k == 'a' || k == 'A') {
                    if (!check_collision(cur_x - 1, cur_y)) cur_x--;
                    break;
                } else if (k == 'd' || k == 'D') {
                    if (!check_collision(cur_x + 1, cur_y)) cur_x++;
                    break;
                } else if (k == 'w' || k == 'W') {
                    rotate_piece();
                    break;
                } else if (k == 's' || k == 'S') {
                    if (!check_collision(cur_x, cur_y + 1)) cur_y++;
                    break;
                } else if (k == ' ') {
                    while (!check_collision(cur_x, cur_y + 1)) cur_y++;
                    break;
                }
            }
            usleep(20000);
        }

        // Natural drop
        if (!check_collision(cur_x, cur_y + 1)) {
            cur_y++;
        } else {
            lock_piece();
            int cl = clear_lines();
            if (cl > 0) {
                lines += cl;
                score += (cl == 1 ? 100 : cl == 2 ? 300 : cl == 3 ? 500 : 800);
                if (delay_ms > 100) delay_ms -= 10;
            }
            spawn_piece();
            if (check_collision(cur_x, cur_y)) {
                // Game Over
                break;
            }
        }
    }

    printf("\n\033[1;31m================ GAME OVER ================\033[0m\n");
    printf("Final Score: \033[1;32m%d\033[0m | Total Lines: \033[1;33m%d\033[0m\n", score, lines);
    printf("Press \033[1;36m'r'\033[0m to Restart or \033[1;31m'q'\033[0m to Return to MicroOS: ");
    fflush(stdout);

    while (1) {
        int k = read_key();
        if (k == 'r' || k == 'R') goto restart_tetris;
        if (k == 'q' || k == 'Q' || k == '\033') break;
        usleep(20000);
    }

    disable_raw_mode();
    printf("\n");
    return 0;
}
