#pragma once

/*
 * board type
 */

#include "move.h"
#include "move_list.h"
#include "piece.h"
#include "square.h"
#include "square_list.h"

typedef struct _cortex_board {
    cortex_piece state[64];
    cortex_move_list legal_moves;
    cortex_move_list move_history;
    int color_to_move;
} cortex_board;

int cortex_board_init(cortex_board* dst);
void cortex_board_draw_types(cortex_board* dst);

int cortex_board_add_attacked_squares(cortex_board* dst, cortex_square sq, cortex_square_list* out);
int cortex_board_add_moving_squares(cortex_board* dst, cortex_square sq, cortex_square_list* out);

/* Get squares attacked by a color. */
int cortex_board_add_attacked_squares_color(cortex_board* dst, cortex_piece_color col, cortex_square_list* out);

int cortex_board_get_color_in_check(cortex_board* dst, cortex_piece_color col);

/* Applies a move without performing post-move legality tests (EG moving into check) */
int cortex_board_apply_move_unchecked_copy(cortex_board* dst, cortex_board* result, cortex_move move);

/*
 * Partially applies a move.
 * Copies the board state, applies a move, analyzes any reminaing move attributes.
 * Returns -1 on invalid arguments or illegal moves.
 */
int cortex_board_complete_move(cortex_board* dst, cortex_move* move, cortex_board* out);

/* Generates the set of legal next moves. Called automatically when a move is performed. */
int cortex_board_gen_legal_moves(cortex_board* dst);

/* Applies a move if it is legal. */
int cortex_board_apply_move(cortex_board* dst, cortex_move move);
