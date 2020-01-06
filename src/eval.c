#include "eval.h"
#include "eval_cache.h"

#include <string.h>
#include <stdio.h>

static cortex_eval _cortex_eval_position_sub(cortex_board* b, int depth);
static float _cortex_clamp(float x);

/*
 * Evaluation function.
 * Recursively
 */
cortex_eval cortex_eval_position(cortex_board* b) {
    return _cortex_eval_position_sub(b, CORTEX_EVAL_DEPTH);
}

cortex_eval _cortex_eval_position_sub(cortex_board* b, int depth) {
    int has_a_move = 0;

    cortex_eval out;
    out.evaluation = 0.0f;
    out.found_mate = 0;
    out.mate_in = 0;
    out.game_over = 0;

    cortex_eval best_next_eval;

    if (!depth) {
        /*
         * Don't look any further.
         * Do a basic evaluation of the position.
         */

        out.evaluation = cortex_eval_immediate(b);
        return out;
    }

    /* Check if there is a cached evaluation at an acceptable depth. */
    int cached_depth;
    cortex_eval cached_eval;

    if (cortex_eval_try_cache(b, &cached_eval, &cached_depth)) {
        /* Got a cache hit. Accept it if it evaluated to the depth we need. */

        if (cached_depth >= depth) {
            return cached_eval;
        }
    }

    /* Cache either missed or was not deep enough. Evaluate from scratch and re-cache the position. */

    /* Iterate through the next legal moves. */
    /* Evaluate each board and find the min-maxed best move. */
    for (int i = 0; i < b->legal_moves.len; ++i) {
        if (depth == CORTEX_EVAL_DEPTH) {
            printf("Evaluating top-level move %d of %d : current best ", i+1, b->legal_moves.len);

            if (has_a_move) {
                cortex_move_print_basic(out.best_move);
            } else {
                printf("\n");
            }
        }

        /* If a move delivers mate, it must be (a) best move. */
        if (b->legal_moves.list[i].move_attr & CORTEX_MOVE_ATTR_MATE) {
            out.found_mate = 1;
            out.mate_in = (b->color_to_move == CORTEX_PIECE_COLOR_WHITE) ? 1 : -1;
            out.best_move = b->legal_moves.list[i];
            return out;
        }

        /* Duplicate the board and evaluate it with a move applied. */
        cortex_board tmp_board;
        memcpy(&tmp_board, b, sizeof tmp_board);
        cortex_board_apply_move(&tmp_board, b->legal_moves.list[i]);

        cortex_eval tmp_eval = _cortex_eval_position_sub(&tmp_board, depth - 1);

        /* Always take the first evaluated move as the best. */
        if (!has_a_move) {
            has_a_move = 1;
            best_next_eval = tmp_eval;
            out.best_move = b->legal_moves.list[i];
        } else {
            /* Compare the evaluation for the color to move. */
            if (cortex_eval_compare(best_next_eval, tmp_eval, b->color_to_move)) {
                best_next_eval = tmp_eval;
                out.best_move = b->legal_moves.list[i];
            }
        }
    }

    if (!has_a_move) {
        out.game_over = 1;
        return out;
    }

    /* Consider the best evaluation. Copy over the value, and if there is a found mate then increment the move count. */
    out.evaluation = best_next_eval.evaluation;

    if (best_next_eval.found_mate) {
        out.found_mate = 1;
        out.mate_in = best_next_eval.found_mate;
        if (out.mate_in < 0) out.mate_in--;
        if (out.mate_in > 0) out.mate_in++;
    }

    /* Cache the new eval if we've made it this far. */
    cortex_eval_cache_insert(b, out, depth);

    return out;
}

int cortex_eval_compare(cortex_eval current_best, cortex_eval candidate_best, cortex_piece_color col) {
    if (col == CORTEX_PIECE_COLOR_WHITE) {
        if (current_best.found_mate && current_best.mate_in > 0) {
            /* White has mate on the board. Minimize moves to mate */
            return (candidate_best.found_mate && candidate_best.mate_in > 0 && candidate_best.mate_in < current_best.mate_in);
        } else if (current_best.found_mate && current_best.mate_in < 0) {
            /* Black has mate on board. If the candidate is a non-mate or the candidate is a longer mate, it is a better move. */
            if (!candidate_best.found_mate) return 1;

            if (candidate_best.mate_in < current_best.mate_in || candidate_best.mate_in > 0) {
                return 1;
            }

            return 0;
        } else {
            /* Current best is not a mate. If a mate for white is found, choose it, otherwise prefer higher evaluation. */
            if (candidate_best.found_mate && candidate_best.mate_in > 0) return 1;
            if (candidate_best.found_mate && candidate_best.mate_in < 0) return 0;

            return (candidate_best.evaluation > current_best.evaluation);
        }
    } else {
        if (current_best.found_mate && current_best.mate_in < 0) {
            return (candidate_best.found_mate && candidate_best.mate_in < 0 && candidate_best.mate_in > current_best.mate_in);
        } else if (current_best.found_mate && current_best.mate_in > 0) {
            if (!candidate_best.found_mate) return 1;

            if (candidate_best.mate_in > current_best.mate_in || candidate_best.mate_in < 0) {
                return 1;
            }

            return 0;
        } else {
            if (candidate_best.found_mate && candidate_best.mate_in < 0) return 1;
            if (candidate_best.found_mate && candidate_best.mate_in > 0) return 0;

            return (candidate_best.evaluation < current_best.evaluation);
        }
    }
}

float cortex_eval_material(cortex_board* b, int total) {
    float sum = 0.0f;

    for (int i = 0; i < 64; ++i) {
        if (CORTEX_PIECE_GET_COLOR(b->state[i]) == CORTEX_PIECE_COLOR_WHITE || total) {
            sum += cortex_eval_piece_value(b->state[i]);
        } else {
            sum -= cortex_eval_piece_value(b->state[i]);
        }
    }

    return sum;
}

float cortex_eval_piece_value(cortex_piece p) {
    if (!p) return 0.0f;
    float out = 0.0f;

    switch (CORTEX_PIECE_GET_TYPE(p)) {
    case CORTEX_PIECE_TYPE_PAWN:
        out = 1.0f;
        break;
    case CORTEX_PIECE_TYPE_BISHOP:
        out = 3.1f;
        break;
    case CORTEX_PIECE_TYPE_KNIGHT:
        out = 3.0f;
        break;
    case CORTEX_PIECE_TYPE_QUEEN:
        out = 6.5f;
        break;
    case CORTEX_PIECE_TYPE_ROOK:
        out = 5.0f;
        break;
    }

    return out;
}

float cortex_eval_immediate(cortex_board* b) {
    float eval = cortex_eval_material(b, 0);
    float opening_factor = cortex_eval_opening_factor(b);

    if (opening_factor > 0.0f) {
        eval += cortex_eval_opening_factor(b) * cortex_eval_opening(b);
    }

    return eval;
}

static float _cortex_clamp(float x) {
    if (x < 0.0f) return 0.0f;
    if (x > 1.0f) return 1.0f;
    return x;
}

float cortex_eval_opening_factor(cortex_board* b) {
    /* The opening factor is the first 20% of total material */
    float tm = (73.5 - cortex_eval_material(b, 1)) / 73.5;

    return _cortex_clamp(tm * 5);
}

float cortex_eval_opening(cortex_board* p) {
    /* Enjoy developing minor pieces. */
    float eval = 0.0f;
    eval += cortex_eval_developed_pieces(p);

    return eval;
}

/* Judge how well pieces are developed for each color. */
float cortex_eval_developed_pieces(cortex_board* p) {
    float eval = 0.0f;

    for (int i = 0; i < 64; ++i) {
        cortex_piece cur = p->state[i];
        cortex_piece_color col = CORTEX_PIECE_GET_COLOR(cur);

        if (!CORTEX_PIECE_IS_MINOR(cur)) continue;

        int rank = CORTEX_SQUARE_RANK(i);

        /* White would like to develop to the third and fourth ranks. Black wants the 6th and 5th. */
        if (col == CORTEX_PIECE_COLOR_WHITE && (rank == 3 || rank == 4)) {
            eval += CORTEX_EVAL_DEVELOPMENT;
        } else if (col == CORTEX_PIECE_COLOR_BLACK && (rank == 6 || rank == 5)) {
            eval -= CORTEX_EVAL_DEVELOPMENT;
        }
    }

    return eval;
}
