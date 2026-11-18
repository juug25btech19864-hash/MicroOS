/*
 * MicroOS Arcade - Snake
 * Standard POSIX terminal game
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/select.h>

#define WIDTH 40
#define HEIGHT 20
#define MAX_SNAKE 800

typedef struct {
    int x, y;
} Point;

static struct termios orig_termios;
static int raw_mode_active = 0;

static void disable_raw_mode(void) {
    if (raw_mode_active) {
        printf("\033[?25h\033[0m\n"); // Show cursor, reset colors
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
        printf("\033[?25l"); // Hide cursor
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
        char seq[3];
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

static int load_high_score(const char *file) {
    FILE *f = fopen(file, "r");
    int score = 0;
    if (f) {
        if (fscanf(f, "%d", &score) != 1) score = 0;
        fclose(f);
    }
    return score;
}

static void save_high_score(const char *file, int score) {
    FILE *f = fopen(file, "w");
    if (f) {
        fprintf(f, "%d\n", score);
        fclose(f);
    }
}

int main(void) {
    enable_raw_mode();
    srand((unsigned int)time(NULL));

    const char *score_file = "/var/games/snake.score";
    int high_score = load_high_score(score_file);
    if (high_score == 0) high_score = load_high_score("/tmp/snake.score");

restart_game:;
    Point snake[MAX_SNAKE];
    int length = 4;
    int dir_x = 1, dir_y = 0;
    int score = 0;
    int game_over = 0;

    snake[0].x = WIDTH / 2;
    snake[0].y = HEIGHT / 2;
    for (int i = 1; i < length; ++i) {
        snake[i].x = snake[0].x - i;
        snake[i].y = snake[0].y;
    }

    Point food;
    int place_food = 1;
    while (place_food) {
        place_food = 0;
        food.x = 1 + rand() % (WIDTH - 2);
        food.y = 1 + rand() % (HEIGHT - 2);
        for (int i = 0; i < length; ++i) {
            if (snake[i].x == food.x && snake[i].y == food.y) {
                place_food = 1;
                break;
            }
        }
    }

    long delay_us = 120000; // 120ms initial speed

    while (!game_over) {
        // Render
        printf("\033[H\033[2J"); // Clear screen
        printf("\033[1;36m=== MICRO ARCADE: SNAKE ===\033[0m\n");
        printf("Score: \033[1;32m%d\033[0m | High Score: \033[1;33m%d\033[0m | Speed: %d\n", 
               score, (score > high_score ? score : high_score), (int)(150000 - delay_us) / 1000);

        // Draw top border
        for (int x = 0; x < WIDTH; ++x) putchar('#');
        putchar('\n');

        for (int y = 1; y < HEIGHT - 1; ++y) {
            putchar('#');
            for (int x = 1; x < WIDTH - 1; ++x) {
                if (x == snake[0].x && y == snake[0].y) {
                    printf("\033[1;32m@\033[0m"); // Head
                } else if (x == food.x && y == food.y) {
                    printf("\033[1;31m*\033[0m"); // Food
                } else {
                    int is_body = 0;
                    for (int i = 1; i < length; ++i) {
                        if (snake[i].x == x && snake[i].y == y) {
                            printf("\033[0;32mo\033[0m"); // Body
                            is_body = 1;
                            break;
                        }
                    }
                    if (!is_body) putchar(' ');
                }
            }
            putchar('#');
            putchar('\n');
        }

        // Draw bottom border
        for (int x = 0; x < WIDTH; ++x) putchar('#');
        putchar('\n');
        printf("Controls: WASD / Arrow Keys | 'q' to Quit\n");
        fflush(stdout);

        // Input & Delay
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = delay_us * 1000L;
        nanosleep(&ts, NULL);

        while (kbhit()) {
            int k = read_key();
            if (k == 'q' || k == 'Q') {
                disable_raw_mode();
                return 0;
            }
            if ((k == 'w' || k == 'W') && dir_y == 0) { dir_x = 0; dir_y = -1; }
            else if ((k == 's' || k == 'S') && dir_y == 0) { dir_x = 0; dir_y = 1; }
            else if ((k == 'a' || k == 'A') && dir_x == 0) { dir_x = -1; dir_y = 0; }
            else if ((k == 'd' || k == 'D') && dir_x == 0) { dir_x = 1; dir_y = 0; }
        }

        // Update position
        int new_x = snake[0].x + dir_x;
        int new_y = snake[0].y + dir_y;

        // Collision with walls
        if (new_x <= 0 || new_x >= WIDTH - 1 || new_y <= 0 || new_y >= HEIGHT - 1) {
            game_over = 1;
            break;
        }

        // Collision with self
        for (int i = 0; i < length; ++i) {
            if (snake[i].x == new_x && snake[i].y == new_y) {
                game_over = 1;
                break;
            }
        }
        if (game_over) break;

        // Check food eat
        int ate = (new_x == food.x && new_y == food.y);
        if (ate) {
            score += 10;
            if (length < MAX_SNAKE - 1) length++;
            if (delay_us > 40000) delay_us -= 2500; // Increase speed

            // Spawn new food
            place_food = 1;
            while (place_food) {
                place_food = 0;
                food.x = 1 + rand() % (WIDTH - 2);
                food.y = 1 + rand() % (HEIGHT - 2);
                for (int i = 0; i < length; ++i) {
                    if (snake[i].x == food.x && snake[i].y == food.y) {
                        place_food = 1;
                        break;
                    }
                }
            }
        }

        // Move body
        for (int i = length - 1; i > 0; --i) {
            snake[i] = snake[i - 1];
        }
        snake[0].x = new_x;
        snake[0].y = new_y;
    }

    // Game Over
    if (score > high_score) {
        high_score = score;
        save_high_score(score_file, high_score);
        save_high_score("/tmp/snake.score", high_score);
    }

    printf("\n\033[1;31m================ GAME OVER ================\033[0m\n");
    printf("Final Score: \033[1;32m%d\033[0m | High Score: \033[1;33m%d\033[0m\n", score, high_score);
    printf("Press \033[1;36m'r'\033[0m to Restart or \033[1;31m'q'\033[0m to Return to MicroOS: ");
    fflush(stdout);

    while (1) {
        int k = read_key();
        if (k == 'r' || k == 'R') goto restart_game;
        if (k == 'q' || k == 'Q' || k == '\033') break;
        usleep(20000);
    }

    disable_raw_mode();
    printf("\n");
    return 0;
}
