/*
 * MicroOS Arcade - Number Guessing Game
 * Terminal number guessing with hints, attempt counters, and difficulty tiers
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void clear_screen(void) {
    printf("\033[H\033[2J");
    fflush(stdout);
}

int main(void) {
    srand((unsigned int)time(NULL));

    while (1) {
        clear_screen();
        printf("\033[1;36m========================================\033[0m\n");
        printf("\033[1;36m    MICRO ARCADE: NUMBER GUESSING      \033[0m\n");
        printf("\033[1;36m========================================\033[0m\n\n");
        printf("Select Difficulty:\n");
        printf("  1. Easy   (Range: 1 - 50,  Max Attempts: 10)\n");
        printf("  2. Medium (Range: 1 - 100, Max Attempts: 7)\n");
        printf("  3. Hard   (Range: 1 - 500, Max Attempts: 9)\n");
        printf("  4. Exit to MicroOS\n\n");
        printf("Choice [1-4]: ");
        fflush(stdout);

        char input[64];
        if (!fgets(input, sizeof(input), stdin)) break;
        int choice = atoi(input);
        if (choice == 4) break;

        int max_val = 100;
        int max_attempts = 7;
        if (choice == 1) { max_val = 50; max_attempts = 10; }
        else if (choice == 3) { max_val = 500; max_attempts = 9; }
        else if (choice != 2) continue;

        int secret = 1 + rand() % max_val;
        int attempts = 0;
        int won = 0;

        clear_screen();
        printf("\033[1;33mI have picked a secret number between 1 and %d.\033[0m\n", max_val);
        printf("You have \033[1;32m%d attempts\033[0m to guess it.\n\n", max_attempts);

        while (attempts < max_attempts) {
            printf("Attempt [%d/%d] Enter your guess: ", attempts + 1, max_attempts);
            fflush(stdout);
            if (!fgets(input, sizeof(input), stdin)) break;
            int guess = atoi(input);
            if (guess <= 0 || guess > max_val) {
                printf("\033[1;31mPlease enter a valid number between 1 and %d!\033[0m\n\n", max_val);
                continue;
            }
            attempts++;

            if (guess == secret) {
                won = 1;
                printf("\n\033[1;32m*** BINGO! You guessed the secret number %d in %d attempts! ***\033[0m\n\n", secret, attempts);
                break;
            } else if (guess < secret) {
                printf("\033[1;36mHIGHER! (Target is greater than %d)\033[0m\n\n", guess);
            } else {
                printf("\033[1;35mLOWER! (Target is smaller than %d)\033[0m\n\n", guess);
            }
        }

        if (!won) {
            printf("\n\033[1;31mOut of attempts! The secret number was %d.\033[0m\n\n", secret);
        }

        printf("Play another round? [y/n]: ");
        fflush(stdout);
        if (!fgets(input, sizeof(input), stdin)) break;
        if (input[0] != 'y' && input[0] != 'Y') break;
    }

    printf("\nReturning to MicroOS.\n");
    return 0;
}
