/*
 * MicroOS Arcade - Tic-Tac-Toe
 * 2-Player & Smart AI Computer Opponent
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void clear_screen(void) {
    printf("\033[H\033[2J");
    fflush(stdout);
}

static void print_board(char board[3][3]) {
    printf("\n");
    for (int r = 0; r < 3; ++r) {
        printf("       %c | %c | %c \n", board[r][0], board[r][1], board[r][2]);
        if (r < 2) printf("      ---+---+---\n");
    }
    printf("\n");
}

static int check_winner(char b[3][3]) {
    // Rows
    for (int r = 0; r < 3; ++r) {
        if (b[r][0] != ' ' && b[r][0] == b[r][1] && b[r][1] == b[r][2]) return b[r][0];
    }
    // Columns
    for (int c = 0; c < 3; ++c) {
        if (b[0][c] != ' ' && b[0][c] == b[1][c] && b[1][c] == b[2][c]) return b[0][c];
    }
    // Diagonals
    if (b[0][0] != ' ' && b[0][0] == b[1][1] && b[1][1] == b[2][2]) return b[0][0];
    if (b[0][2] != ' ' && b[0][2] == b[1][1] && b[1][1] == b[2][0]) return b[0][2];

    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            if (b[r][c] == ' ') return 0; // In progress
        }
    }
    return 'D'; // Draw
}

// Minimax algorithm for computer move
static int minimax(char b[3][3], int depth, int is_ai) {
    int score = check_winner(b);
    if (score == 'O') return 10 - depth;
    if (score == 'X') return depth - 10;
    if (score == 'D') return 0;

    if (is_ai) {
        int best = -1000;
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                if (b[r][c] == ' ') {
                    b[r][c] = 'O';
                    int val = minimax(b, depth + 1, 0);
                    b[r][c] = ' ';
                    if (val > best) best = val;
                }
            }
        }
        return best;
    } else {
        int best = 1000;
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                if (b[r][c] == ' ') {
                    b[r][c] = 'X';
                    int val = minimax(b, depth + 1, 1);
                    b[r][c] = ' ';
                    if (val < best) best = val;
                }
            }
        }
        return best;
    }
}

static void find_best_move(char b[3][3], int *best_r, int *best_c) {
    int best_val = -1000;
    *best_r = -1;
    *best_c = -1;
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            if (b[r][c] == ' ') {
                b[r][c] = 'O';
                int move_val = minimax(b, 0, 0);
                b[r][c] = ' ';
                if (move_val > best_val) {
                    *best_r = r;
                    *best_c = c;
                    best_val = move_val;
                }
            }
        }
    }
}

int main(void) {
    srand((unsigned int)time(NULL));

    while (1) {
        clear_screen();
        printf("\033[1;36m========================================\033[0m\n");
        printf("\033[1;36m       MICRO ARCADE: TIC-TAC-TOE        \033[0m\n");
        printf("\033[1;36m========================================\033[0m\n\n");
        printf("Mode Selection:\n");
        printf("  1. Single Player vs Smart AI (Computer = 'O')\n");
        printf("  2. Two-Player Pass & Play\n");
        printf("  3. Exit to MicroOS\n\n");
        printf("Choice [1-3]: ");
        fflush(stdout);

        char input[64];
        if (!fgets(input, sizeof(input), stdin)) break;
        int mode = atoi(input);
        if (mode == 3) break;
        if (mode != 1 && mode != 2) continue;

        char board[3][3] = {
            {' ', ' ', ' '},
            {' ', ' ', ' '},
            {' ', ' ', ' '}
        };

        char cur_turn = 'X';
        int game_state = 0;

        while ((game_state = check_winner(board)) == 0) {
            clear_screen();
            printf("\033[1;36m=== MICRO ARCADE: TIC-TAC-TOE ===\033[0m\n");
            printf("Positions correspond to numbers 1 to 9:\n");
            printf("  1 | 2 | 3 \n ---+---+---\n  4 | 5 | 6 \n ---+---+---\n  7 | 8 | 9 \n");
            print_board(board);

            if (mode == 1 && cur_turn == 'O') {
                printf("\033[1;33mComputer ('O') is thinking...\033[0m\n");
                int br, bc;
                find_best_move(board, &br, &bc);
                board[br][bc] = 'O';
                cur_turn = 'X';
            } else {
                printf("Player \033[1;32m%c\033[0m's Turn. Enter position [1-9] or '0' to quit: ", cur_turn);
                fflush(stdout);
                if (!fgets(input, sizeof(input), stdin)) goto quit_game;
                int pos = atoi(input);
                if (pos == 0) goto quit_game;
                if (pos < 1 || pos > 9) continue;
                int r = (pos - 1) / 3;
                int c = (pos - 1) % 3;
                if (board[r][c] != ' ') {
                    printf("\033[1;31mCell already occupied! Choose another.\033[0m\n");
                    continue;
                }
                board[r][c] = cur_turn;
                cur_turn = (cur_turn == 'X' ? 'O' : 'X');
            }
        }

        clear_screen();
        printf("\033[1;36m=== FINAL BOARD ===\033[0m\n");
        print_board(board);

        if (game_state == 'D') {
            printf("\033[1;33m*** It's a DRAW! Well played! ***\033[0m\n\n");
        } else {
            printf("\033[1;32m*** PLAYER %c WINS! CONGRATULATIONS! ***\033[0m\n\n", game_state);
        }

        printf("Play another game? [y/n]: ");
        fflush(stdout);
        if (!fgets(input, sizeof(input), stdin)) break;
        if (input[0] != 'y' && input[0] != 'Y') break;
    }

quit_game:
    printf("\nReturning to MicroOS.\n");
    return 0;
}
