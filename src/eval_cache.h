#pragma once

/*
 * Evaluation cache
 * The evaluation cache resides in static memory.
 * If multithreading is implemented in the future the code will need to be modified.
 */

#include "eval.h"

#define CORTEX_EVAL_CACHE_SIZE 1024

typedef struct _cortex_eval_cache_entry {
    cortex_piece state[64];
    cortex_piece_color to_move;
    cortex_eval eval;
    int filled;
} cortex_eval_cache_entry;

/* Returns 1 if the position was located in the cache, and fills *out with the evaluation if it is. */
int cortex_eval_try_cache(cortex_board* b, cortex_eval* out);

void cortex_eval_cache_insert(cortex_board* b, cortex_eval eval);
