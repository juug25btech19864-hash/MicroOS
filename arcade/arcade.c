/*
 * MicroOS Arcade Launcher
 * Interactive menu launcher for all 8 built-in terminal games
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void clear_screen(void) {
    printf("\033[H\033[2J");
    fflush(stdout);
}

static void show_menu(void) {
    clear_screen();
    printf("\033[1;36m================================\033[0m\n");
    printf("\033[1;32m         MICRO ARCADE           \033[0m\n");
    printf("\033[1;36m================================\033[0m\n\n");
    printf("  \033[1;33m1.\033[0m Snake\n");
    printf("  \033[1;33m2.\033[0m Tetris\n");
    printf("  \033[1;33m3.\033[0m Pong\n");
    printf("  \033[1;33m4.\033[0m Minesweeper\n");
    printf("  \033[1;33m5.\033[0m 2048\n");
    printf("  \033[1;33m6.\033[0m Number Guessing\n");
    printf("  \033[1;33m7.\033[0m Tic-Tac-Toe\n");
    printf("  \033[1;33m8.\033[0m Space Invaders\n");
    printf("  \033[1;31m9.\033[0m Exit to Shell\n\n");
    printf("Select game [1-9]: ");
    fflush(stdout);
}

int main(void) {
    char input[64];

    while (1) {
        show_menu();
        if (!fgets(input, sizeof(input), stdin)) break;

        int choice = atoi(input);
        const char *cmd = NULL;

        switch (choice) {
            case 1: cmd = "snake"; break;
            case 2: cmd = "tetris"; break;
            case 3: cmd = "pong"; break;
            case 4: cmd = "mines"; break;
            case 5: cmd = "2048"; break;
            case 6: cmd = "guess"; break;
            case 7: cmd = "tictactoe"; break;
            case 8: cmd = "spaceinvaders"; break;
            case 9:
                clear_screen();
                printf("Returning to MicroOS shell.\n");
                return 0;
            default:
                continue;
        }

        if (cmd) {
            // First check local /usr/games/<cmd> or fallback to PATH
            char full_cmd[128];
            snprintf(full_cmd, sizeof(full_cmd), "which %s >/dev/null 2>&1 && %s || /usr/games/%s", cmd, cmd, cmd);
            int ret = system(full_cmd);
            (void)ret;
        }
    }

    return 0;
}
