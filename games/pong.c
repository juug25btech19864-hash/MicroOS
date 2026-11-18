/*
 * MicroOS Arcade - Pong
 * Real-time terminal Pong game (Player vs AI / 2-Player)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <signal.h>
#include <sys/select.h>

#define WIDTH 50
#define HEIGHT 18
#define PADDLE_SIZE 4

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
                case 'A': return 'k'; // Up
                case 'B': return 'm'; // Down
            }
        }
        return '\033';
    }
    return c;
}

int main(int argc, char **argv) {
    int two_player = (argc > 1 && strcmp(argv[1], "2p") == 0);
    enable_raw_mode();
    srand((unsigned int)time(NULL));

restart_pong:;
    int p1_y = (HEIGHT - PADDLE_SIZE) / 2;
    int p2_y = (HEIGHT - PADDLE_SIZE) / 2;
    float ball_x = WIDTH / 2.0f;
    float ball_y = HEIGHT / 2.0f;
    float ball_vx = 1.0f;
    float ball_vy = 0.5f;
    int score1 = 0, score2 = 0;
    const int WIN_SCORE = 5;

    while (score1 < WIN_SCORE && score2 < WIN_SCORE) {
        // Render
        printf("\033[H\033[2J");
        printf("\033[1;36m=== MICRO ARCADE: PONG ===\033[0m\n");
        printf("Player 1: \033[1;32m%d\033[0m  |  %s: \033[1;33m%d\033[0m (First to %d)\n",
               score1, two_player ? "Player 2" : "Computer AI", score2, WIN_SCORE);

        // Top wall
        for (int x = 0; x < WIDTH + 2; ++x) putchar('=');
        putchar('\n');

        int iball_x = (int)ball_x;
        int iball_y = (int)ball_y;

        for (int y = 0; y < HEIGHT; ++y) {
            putchar('|');
            for (int x = 0; x < WIDTH; ++x) {
                if (x == 1 && y >= p1_y && y < p1_y + PADDLE_SIZE) {
                    printf("\033[1;32m#\033[0m"); // Paddle 1
                } else if (x == WIDTH - 2 && y >= p2_y && y < p2_y + PADDLE_SIZE) {
                    printf("\033[1;33m#\033[0m"); // Paddle 2
                } else if (x == iball_x && y == iball_y) {
                    printf("\033[1;37mO\033[0m"); // Ball
                } else if (x == WIDTH / 2) {
                    printf("\033[0;37m:\033[0m"); // Net
                } else {
                    putchar(' ');
                }
            }
            putchar('|');
            putchar('\n');
        }

        // Bottom wall
        for (int x = 0; x < WIDTH + 2; ++x) putchar('=');
        putchar('\n');
        if (two_player) {
            printf("Controls: P1: [W/S] | P2: [Up/Down] | [Q] Quit\n");
        } else {
            printf("Controls: [W/S] Move Paddle | [Q] Quit\n");
        }
        fflush(stdout);

        // Input
        while (kbhit()) {
            int k = read_key();
            if (k == 'q' || k == 'Q') {
                disable_raw_mode();
                return 0;
            }
            if (k == 'w' || k == 'W') { if (p1_y > 0) p1_y--; }
            if (k == 's' || k == 'S') { if (p1_y < HEIGHT - PADDLE_SIZE) p1_y++; }
            if (two_player) {
                if (k == 'k' || k == 'K') { if (p2_y > 0) p2_y--; }
                if (k == 'm' || k == 'M') { if (p2_y < HEIGHT - PADDLE_SIZE) p2_y++; }
            }
        }

        // AI movement
        if (!two_player) {
            int target_y = (int)ball_y - (PADDLE_SIZE / 2);
            if (rand() % 100 < 80) { // 80% accuracy
                if (p2_y < target_y && p2_y < HEIGHT - PADDLE_SIZE) p2_y++;
                else if (p2_y > target_y && p2_y > 0) p2_y--;
            }
        }

        // Ball movement
        ball_x += ball_vx;
        ball_y += ball_vy;

        // Top/Bottom bounce
        if (ball_y <= 0) { ball_y = 0; ball_vy = -ball_vy; }
        if (ball_y >= HEIGHT - 1) { ball_y = HEIGHT - 1; ball_vy = -ball_vy; }

        // Paddle 1 collision
        if (ball_x <= 2 && ball_x >= 1) {
            if ((int)ball_y >= p1_y && (int)ball_y < p1_y + PADDLE_SIZE) {
                ball_vx = -ball_vx;
                ball_x = 2;
                // Add spin
                float diff = (ball_y - (p1_y + PADDLE_SIZE / 2.0f)) * 0.3f;
                ball_vy += diff;
            }
        }

        // Paddle 2 collision
        if (ball_x >= WIDTH - 3 && ball_x <= WIDTH - 2) {
            if ((int)ball_y >= p2_y && (int)ball_y < p2_y + PADDLE_SIZE) {
                ball_vx = -ball_vx;
                ball_x = WIDTH - 3;
                float diff = (ball_y - (p2_y + PADDLE_SIZE / 2.0f)) * 0.3f;
                ball_vy += diff;
            }
        }

        // Goal check
        if (ball_x < 0) {
            score2++;
            ball_x = WIDTH / 2.0f;
            ball_y = HEIGHT / 2.0f;
            ball_vx = 1.0f;
            ball_vy = 0.5f;
            usleep(500000);
        } else if (ball_x >= WIDTH) {
            score1++;
            ball_x = WIDTH / 2.0f;
            ball_y = HEIGHT / 2.0f;
            ball_vx = -1.0f;
            ball_vy = 0.5f;
            usleep(500000);
        }

        usleep(45000); // Frame delay
    }

    printf("\n\033[1;32m================ MATCH FINISHED ================\033[0m\n");
    if (score1 >= WIN_SCORE) printf("\033[1;32mPLAYER 1 WINS!\033[0m (%d - %d)\n", score1, score2);
    else printf("\033[1;33m%s WINS!\033[0m (%d - %d)\n", two_player ? "PLAYER 2" : "COMPUTER AI", score2, score1);
    printf("Press \033[1;36m'r'\033[0m to Play Again or \033[1;31m'q'\033[0m to Return to MicroOS: ");
    fflush(stdout);

    while (1) {
        int k = read_key();
        if (k == 'r' || k == 'R') goto restart_pong;
        if (k == 'q' || k == 'Q' || k == '\033') break;
        usleep(20000);
    }

    disable_raw_mode();
    printf("\n");
    return 0;
}
