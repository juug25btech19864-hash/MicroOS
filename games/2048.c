/*
 * MicroOS Arcade - 2048
 * Classic 2048 sliding block puzzle
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <signal.h>

#define SIZE 4

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
    raw.c_cc[VMIN] = 1;
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
    if (read(STDIN_FILENO, &c, 1) != 1) return 0;
    if (c == '\033') {
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\033';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\033';
        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return 'w';
                case 'B': return 's';
                case 'C': return 'd';
                case 'D': return 'a';
            }
        }
        return '\033';
    }
    return c;
}

static int grid[SIZE][SIZE];
static int score = 0;

static const char* get_color(int val) {
    switch (val) {
        case 2:    return "\033[0;37m";
        case 4:    return "\033[1;37m";
        case 8:    return "\033[0;33m";
        case 16:   return "\033[1;33m";
        case 32:   return "\033[0;31m";
        case 64:   return "\033[1;31m";
        case 128:  return "\033[1;36m";
        case 256:  return "\033[0;32m";
        case 512:  return "\033[1;32m";
        case 1024: return "\033[0;35m";
        case 2048: return "\033[1;35m";
        default:   return "\033[1;34m";
    }
}

static void spawn_tile(void) {
    int empty_r[SIZE * SIZE], empty_c[SIZE * SIZE];
    int count = 0;
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            if (grid[r][c] == 0) {
                empty_r[count] = r;
                empty_c[count] = c;
                count++;
            }
        }
    }
    if (count > 0) {
        int idx = rand() % count;
        grid[empty_r[idx]][empty_c[idx]] = (rand() % 10 == 0) ? 4 : 2;
    }
}

static int move_left(void) {
    int moved = 0;
    for (int r = 0; r < SIZE; ++r) {
        int line[SIZE] = {0};
        int target = 0;
        int merged[SIZE] = {0};

        for (int c = 0; c < SIZE; ++c) {
            if (grid[r][c] != 0) {
                if (target > 0 && line[target - 1] == grid[r][c] && !merged[target - 1]) {
                    line[target - 1] *= 2;
                    score += line[target - 1];
                    merged[target - 1] = 1;
                    moved = 1;
                } else {
                    line[target] = grid[r][c];
                    if (target != c) moved = 1;
                    target++;
                }
            }
        }
        for (int c = 0; c < SIZE; ++c) grid[r][c] = line[c];
    }
    return moved;
}

static void rotate_grid_clockwise(void) {
    int temp[SIZE][SIZE];
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            temp[c][SIZE - 1 - r] = grid[r][c];
        }
    }
    memcpy(grid, temp, sizeof(temp));
}

static int make_move(char dir) {
    int moved = 0;
    if (dir == 'a') {
        moved = move_left();
    } else if (dir == 's') {
        rotate_grid_clockwise();
        rotate_grid_clockwise();
        rotate_grid_clockwise();
        moved = move_left();
        rotate_grid_clockwise();
    } else if (dir == 'd') {
        rotate_grid_clockwise();
        rotate_grid_clockwise();
        moved = move_left();
        rotate_grid_clockwise();
        rotate_grid_clockwise();
    } else if (dir == 'w') {
        rotate_grid_clockwise();
        moved = move_left();
        rotate_grid_clockwise();
        rotate_grid_clockwise();
        rotate_grid_clockwise();
    }
    return moved;
}

static int can_move(void) {
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            if (grid[r][c] == 0) return 1;
            if (c + 1 < SIZE && grid[r][c] == grid[r][c + 1]) return 1;
            if (r + 1 < SIZE && grid[r][c] == grid[r + 1][c]) return 1;
        }
    }
    return 0;
}

int main(void) {
    enable_raw_mode();
    srand((unsigned int)time(NULL));

restart_2048:;
    memset(grid, 0, sizeof(grid));
    score = 0;
    spawn_tile();
    spawn_tile();

    while (1) {
        printf("\033[H\033[2J");
        printf("\033[1;36m=== MICRO ARCADE: 2048 ===\033[0m\n");
        printf("Score: \033[1;32m%d\033[0m\n\n", score);

        printf("+------+------+------+------+\n");
        for (int r = 0; r < SIZE; ++r) {
            for (int c = 0; c < SIZE; ++c) {
                printf("|");
                int val = grid[r][c];
                if (val > 0) {
                    printf("%s%5d \033[0m", get_color(val), val);
                } else {
                    printf("      ");
                }
            }
            printf("|\n+------+------+------+------+\n");
        }
        printf("\nControls: [WASD/Arrows] Slide | [Q] Quit\n");
        fflush(stdout);

        if (!can_move()) break;

        int k = read_key();
        if (k == 'q' || k == 'Q') {
            disable_raw_mode();
            return 0;
        }
        if (k == 'w' || k == 's' || k == 'a' || k == 'd') {
            if (make_move(k)) {
                spawn_tile();
            }
        }
    }

    printf("\n\033[1;31m================ GAME OVER ================\033[0m\n");
    printf("Final Score: \033[1;32m%d\033[0m\n", score);
    printf("Press \033[1;36m'r'\033[0m to Play Again or \033[1;31m'q'\033[0m to Return to MicroOS: ");
    fflush(stdout);

    while (1) {
        int k = read_key();
        if (k == 'r' || k == 'R') goto restart_2048;
        if (k == 'q' || k == 'Q' || k == '\033') break;
        usleep(20000);
    }

    disable_raw_mode();
    printf("\n");
    return 0;
}
