#pragma once

#include "move.h"
#include "board.h"

#define CORTEX_EVAL_DEPTH 3

/*
 * Generic importance for phase-specific evaluations.
 * Scales all evaluation bonuses for the respective stage.
 */
#define CORTEX_EVAL_OPENING_SCALE 1.0f
#define CORTEX_EVAL_MIDDLEGAME_SCALE 1.0f
#define CORTEX_EVAL_ENDGAME_SCALE 1.0f

#define CORTEX_EVAL_DEVELOPMENT 1.5f

typedef struct _cortex_eval {
    float evaluation;
    int found_mate;
    int mate_in;
    int game_over;
    cortex_move best_move;
} cortex_eval;

/*
 * Cortex evaluation function.
 * Evaluates a position synchronously to the full depth and computes the best move for
 * the color to move.
 */
cortex_eval cortex_eval_position(cortex_board* b);

float cortex_eval_opening(cortex_board* b);
float cortex_eval_middlegame(cortex_board* b);
float cortex_eval_endgame(cortex_board* b);

/* Get the factor opinions of what stage the game is in. */
float cortex_eval_opening_factor(cortex_board* b);
float cortex_eval_middlegame_factor(cortex_board* b);
float cortex_eval_endgame_factor(cortex_board* b);

float cortex_eval_immediate(cortex_board* b);

/* Get current material difference. (white-black) */
float cortex_eval_material(cortex_board* b, int total);

/* Get value of a piece. Always positive. */
float cortex_eval_piece_value(cortex_piece p);

float cortex_eval_developed_pieces(cortex_board* b);

/* Returns nonzero if the candidate is a better evalation for <col> than the current best. */
int cortex_eval_compare(cortex_eval current_best, cortex_eval candidate_best, cortex_piece_color col);
