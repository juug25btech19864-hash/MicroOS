/*
 * MicroOS Arcade - Minesweeper
 * Classic terminal minesweeper with cursor navigation & flagging
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <signal.h>
#include <sys/select.h>

#define ROWS 9
#define COLS 9
#define NUM_MINES 10

typedef struct {
    int has_mine;
    int is_revealed;
    int is_flagged;
    int adj_mines;
} Cell;

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

static int read_key(void) {
    char c;
    while (read(STDIN_FILENO, &c, 1) != 1) {
        usleep(10000);
    }
    if (c == '\033') {
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\033';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\033';
        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return 'w'; // Up
                case 'B': return 's'; // Down
                case 'C': return 'd'; // Right
                case 'D': return 'a'; // Left
            }
        }
        return '\033';
    }
    return c;
}

static Cell grid[ROWS][COLS];
static int cursor_r = 0, cursor_c = 0;
static int first_move = 1;

static void place_mines(int safe_r, int safe_c) {
    int placed = 0;
    while (placed < NUM_MINES) {
        int r = rand() % ROWS;
        int c = rand() % COLS;
        if ((r >= safe_r - 1 && r <= safe_r + 1) && (c >= safe_c - 1 && c <= safe_c + 1)) continue;
        if (!grid[r][c].has_mine) {
            grid[r][c].has_mine = 1;
            placed++;
        }
    }
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            if (grid[r][c].has_mine) continue;
            int count = 0;
            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {
                    int nr = r + dr, nc = c + dc;
                    if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS) {
                        if (grid[nr][nc].has_mine) count++;
                    }
                }
            }
            grid[r][c].adj_mines = count;
        }
    }
}

static void reveal_cell(int r, int c) {
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return;
    if (grid[r][c].is_revealed || grid[r][c].is_flagged) return;

    grid[r][c].is_revealed = 1;
    if (grid[r][c].adj_mines == 0 && !grid[r][c].has_mine) {
        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr != 0 || dc != 0) reveal_cell(r + dr, c + dc);
            }
        }
    }
}

static int check_win(void) {
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            if (!grid[r][c].has_mine && !grid[r][c].is_revealed) return 0;
        }
    }
    return 1;
}

int main(void) {
    enable_raw_mode();
    srand((unsigned int)time(NULL));

restart_mines:;
    memset(grid, 0, sizeof(grid));
    cursor_r = ROWS / 2;
    cursor_c = COLS / 2;
    first_move = 1;
    int game_over = 0, win = 0;

    while (!game_over && !win) {
        printf("\033[H\033[2J");
        printf("\033[1;36m=== MICRO ARCADE: MINESWEEPER ===\033[0m\n");
        printf("Mines: %d | Size: %dx%d\n\n", NUM_MINES, ROWS, COLS);

        printf("    ");
        for (int c = 0; c < COLS; ++c) printf("%d ", c + 1);
        printf("\n   +");
        for (int c = 0; c < COLS; ++c) printf("--");
        printf("+\n");

        for (int r = 0; r < ROWS; ++r) {
            printf(" %d |", r + 1);
            for (int c = 0; c < COLS; ++c) {
                int is_cur = (r == cursor_r && c == cursor_c);
                if (is_cur) printf("\033[7m"); // Inverse video for cursor

                if (grid[r][c].is_flagged) {
                    printf("\033[1;33mF\033[0m ");
                } else if (!grid[r][c].is_revealed) {
                    printf(". ");
                } else if (grid[r][c].has_mine) {
                    printf("\033[1;31m*\033[0m ");
                } else if (grid[r][c].adj_mines == 0) {
                    printf("  ");
                } else {
                    const char *colors[] = { "", "\033[1;34m", "\033[1;32m", "\033[1;31m", "\033[1;35m", "\033[1;33m" };
                    int ci = grid[r][c].adj_mines <= 5 ? grid[r][c].adj_mines : 5;
                    printf("%s%d\033[0m ", colors[ci], grid[r][c].adj_mines);
                }
                if (is_cur) printf("\033[0m");
            }
            printf("|\n");
        }
        printf("   +");
        for (int c = 0; c < COLS; ++c) printf("--");
        printf("+\n");
        printf("Controls: [WASD/Arrows] Move | [Space/Enter] Dig | [F] Flag | [Q] Quit\n");
        fflush(stdout);

        int k = read_key();
        if (k == 'q' || k == 'Q') {
            disable_raw_mode();
            return 0;
        } else if (k == 'w' || k == 'W') { if (cursor_r > 0) cursor_r--; }
        else if (k == 's' || k == 'S') { if (cursor_r < ROWS - 1) cursor_r++; }
        else if (k == 'a' || k == 'A') { if (cursor_c > 0) cursor_c--; }
        else if (k == 'd' || k == 'D') { if (cursor_c < COLS - 1) cursor_c++; }
        else if (k == 'f' || k == 'F') {
            if (!grid[cursor_r][cursor_c].is_revealed) {
                grid[cursor_r][cursor_c].is_flagged = !grid[cursor_r][cursor_c].is_flagged;
            }
        } else if (k == ' ' || k == '\r' || k == '\n') {
            if (grid[cursor_r][cursor_c].is_flagged) continue;
            if (first_move) {
                place_mines(cursor_r, cursor_c);
                first_move = 0;
            }
            if (grid[cursor_r][cursor_c].has_mine) {
                game_over = 1;
                // Reveal all mines
                for (int r = 0; r < ROWS; ++r) {
                    for (int c = 0; c < COLS; ++c) {
                        if (grid[r][c].has_mine) grid[r][c].is_revealed = 1;
                    }
                }
            } else {
                reveal_cell(cursor_r, cursor_c);
                if (check_win()) win = 1;
            }
        }
    }

    // Final board display
    printf("\033[H\033[2J");
    printf("\033[1;36m=== MICRO ARCADE: MINESWEEPER ===\033[0m\n\n");
    for (int r = 0; r < ROWS; ++r) {
        printf(" |");
        for (int c = 0; c < COLS; ++c) {
            if (grid[r][c].has_mine) printf("\033[1;31m* \033[0m");
            else if (!grid[r][c].is_revealed) printf(". ");
            else if (grid[r][c].adj_mines == 0) printf("  ");
            else printf("%d ", grid[r][c].adj_mines);
        }
        printf("|\n");
    }

    if (win) {
        printf("\n\033[1;32m*** CONGRATULATIONS! ALL MINES CLEARED! ***\033[0m\n");
    } else {
        printf("\n\033[1;31m*** BOOM! YOU HIT A MINE! GAME OVER! ***\033[0m\n");
    }
    printf("Press \033[1;36m'r'\033[0m to Play Again or \033[1;31m'q'\033[0m to Return to MicroOS: ");
    fflush(stdout);

    while (1) {
        int k = read_key();
        if (k == 'r' || k == 'R') goto restart_mines;
        if (k == 'q' || k == 'Q' || k == '\033') break;
        usleep(20000);
    }

    disable_raw_mode();
    printf("\n");
    return 0;
}
