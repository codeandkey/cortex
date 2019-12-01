#include "board.h"
#include "eval.h"

#include <stdio.h>

int main(int argc, char** argv) {
    cortex_board b;
    cortex_board_init(&b);

    while (1) {
        cortex_board_draw_types(&b);

        printf("Evaluating position..\n");
        cortex_eval eval = cortex_eval_position(&b);
        printf("Decided on best move ");
        cortex_move_print_basic(eval.best_move);
        printf(" with current evaluation ");

        if (eval.found_mate) {
            printf("#%d\n", eval.mate_in);
        } else {
            printf("%f\n", eval.evaluation);
        }

        const char* prompt = "black move: ";
        if (b.color_to_move == CORTEX_PIECE_COLOR_WHITE) {
            prompt = "white move: ";
        }

        char mode = getchar();

        if (mode == 'l') {
            printf("Legal moves:\n");
            cortex_move_list_print(&b.legal_moves);
        } else if (mode == '?') {
            printf("manual move: %s", prompt);
            cortex_square from = cortex_square_read();
            cortex_square to = cortex_square_read();

            printf("move: ");
            cortex_square_print(from);
            printf(" to ");
            cortex_square_print(to);
            printf("\n");

            cortex_move move;
            
            if (cortex_move_list_get(&b.legal_moves, from, to, &move)) {
                printf("Invalid move: ");
                cortex_square_print(from);
                printf(" to ");
                cortex_square_print(to);
                continue;
            }

            printf("applying matched move ");
            cortex_move_print_basic(move);

            cortex_board_apply_move(&b, move);
        } else if (mode == '.') {
            printf("applying best move: ");
            cortex_move_print_basic(eval.best_move);
            cortex_board_apply_move(&b, eval.best_move);
        }
    }



    return 0;
}
