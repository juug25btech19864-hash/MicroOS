/*
 * MicroOS Arcade - Space Invaders
 * Real-time ASCII space shooter
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <signal.h>
#include <sys/select.h>

#define WIDTH 44
#define HEIGHT 18
#define MAX_ALIENS 20
#define MAX_BULLETS 10

typedef struct {
    int x, y;
    int alive;
} Alien;

typedef struct {
    int x, y;
    int active;
} Bullet;

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
                case 'A': return ' '; // Up / Shoot
                case 'C': return 'd'; // Right
                case 'D': return 'a'; // Left
            }
        }
        return '\033';
    }
    return c;
}

int main(void) {
    enable_raw_mode();
    srand((unsigned int)time(NULL));

restart_invaders:;
    int player_x = WIDTH / 2;
    Alien aliens[MAX_ALIENS];
    int alien_count = 0;

    for (int r = 0; r < 2; ++r) {
        for (int c = 0; c < 10; ++c) {
            aliens[alien_count].x = 6 + c * 3;
            aliens[alien_count].y = 2 + r * 2;
            aliens[alien_count].alive = 1;
            alien_count++;
        }
    }

    Bullet bullets[MAX_BULLETS];
    memset(bullets, 0, sizeof(bullets));

    int score = 0;
    int alien_dir = 1;
    int alien_tick = 0;
    int game_over = 0;
    int win = 0;

    while (!game_over && !win) {
        // Render
        printf("\033[H\033[2J");
        printf("\033[1;36m=== MICRO ARCADE: SPACE INVADERS ===\033[0m\n");
        printf("Score: \033[1;32m%d\033[0m | Aliens Left: \033[1;31m%d\033[0m\n", score, alien_count);

        for (int x = 0; x < WIDTH + 2; ++x) putchar('=');
        putchar('\n');

        for (int y = 0; y < HEIGHT; ++y) {
            putchar('|');
            for (int x = 0; x < WIDTH; ++x) {
                if (y == HEIGHT - 1 && x == player_x) {
                    printf("\033[1;32m/A\\\033[0m"); // Player ship (3 chars, adjust width)
                    x += 2;
                } else {
                    int drawn = 0;
                    // Check bullet
                    for (int b = 0; b < MAX_BULLETS; ++b) {
                        if (bullets[b].active && bullets[b].x == x && bullets[b].y == y) {
                            printf("\033[1;33m|\033[0m");
                            drawn = 1;
                            break;
                        }
                    }
                    if (drawn) continue;

                    // Check alien
                    for (int a = 0; a < MAX_ALIENS; ++a) {
                        if (aliens[a].alive && aliens[a].x == x && aliens[a].y == y) {
                            printf("\033[1;35mW\033[0m");
                            drawn = 1;
                            break;
                        }
                    }
                    if (!drawn) putchar(' ');
                }
            }
            putchar('|');
            putchar('\n');
        }

        for (int x = 0; x < WIDTH + 2; ++x) putchar('=');
        putchar('\n');
        printf("Controls: [A/D / Left/Right] Move | [Space/W] Shoot | [Q] Quit\n");
        fflush(stdout);

        // Input
        while (kbhit()) {
            int k = read_key();
            if (k == 'q' || k == 'Q') {
                disable_raw_mode();
                return 0;
            }
            if (k == 'a' || k == 'A') { if (player_x > 2) player_x -= 2; }
            if (k == 'd' || k == 'D') { if (player_x < WIDTH - 4) player_x += 2; }
            if (k == ' ' || k == 'w' || k == 'W') {
                for (int b = 0; b < MAX_BULLETS; ++b) {
                    if (!bullets[b].active) {
                        bullets[b].active = 1;
                        bullets[b].x = player_x + 1;
                        bullets[b].y = HEIGHT - 2;
                        break;
                    }
                }
            }
        }

        // Bullet physics
        for (int b = 0; b < MAX_BULLETS; ++b) {
            if (bullets[b].active) {
                bullets[b].y--;
                if (bullets[b].y < 0) {
                    bullets[b].active = 0;
                    continue;
                }
                // Collision with aliens
                for (int a = 0; a < MAX_ALIENS; ++a) {
                    if (aliens[a].alive && (aliens[a].x == bullets[b].x || aliens[a].x == bullets[b].x - 1 || aliens[a].x == bullets[b].x + 1) && aliens[a].y == bullets[b].y) {
                        aliens[a].alive = 0;
                        bullets[b].active = 0;
                        score += 50;
                        alien_count--;
                        if (alien_count <= 0) win = 1;
                        break;
                    }
                }
            }
        }

        // Alien movement
        alien_tick++;
        if (alien_tick >= 4) {
            alien_tick = 0;
            int change_dir = 0;
            for (int a = 0; a < MAX_ALIENS; ++a) {
                if (aliens[a].alive) {
                    if ((aliens[a].x >= WIDTH - 3 && alien_dir > 0) || (aliens[a].x <= 2 && alien_dir < 0)) {
                        change_dir = 1;
                        break;
                    }
                }
            }
            if (change_dir) {
                alien_dir = -alien_dir;
                for (int a = 0; a < MAX_ALIENS; ++a) {
                    if (aliens[a].alive) {
                        aliens[a].y++;
                        if (aliens[a].y >= HEIGHT - 2) game_over = 1;
                    }
                }
            } else {
                for (int a = 0; a < MAX_ALIENS; ++a) {
                    if (aliens[a].alive) aliens[a].x += alien_dir;
                }
            }
        }

        usleep(60000);
    }

    printf("\n");
    if (win) {
        printf("\033[1;32m*** VICTORY! EARTH DEFENDED! CONGRATULATIONS! ***\033[0m\n");
    } else {
        printf("\033[1;31m*** INVASION SUCCESSFUL! GAME OVER! ***\033[0m\n");
    }
    printf("Final Score: \033[1;32m%d\033[0m\n", score);
    printf("Press \033[1;36m'r'\033[0m to Play Again or \033[1;31m'q'\033[0m to Return to MicroOS: ");
    fflush(stdout);

    while (1) {
        int k = read_key();
        if (k == 'r' || k == 'R') goto restart_invaders;
        if (k == 'q' || k == 'Q' || k == '\033') break;
        usleep(20000);
    }

    disable_raw_mode();
    printf("\n");
    return 0;
}
