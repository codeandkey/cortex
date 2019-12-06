#include "eval_cache.h"
#include "xxhash.h"
#include "log.h"

#include <string.h>

static cortex_eval_cache_entry _cortex_eval_cache[CORTEX_EVAL_CACHE_SIZE];

static cortex_eval_cache_entry* _cortex_eval_cache_get_dst(cortex_board* b);

int cortex_eval_try_cache(cortex_board* b, cortex_eval* out, int *out_depth) {
    cortex_eval_cache_entry* dst = _cortex_eval_cache_get_dst(b);

    if (!dst->game.len) return 0; /* don't cache empty games */

    if (cortex_move_list_equals(&b->move_history, &dst->game)) {
        *out = dst->eval;
        *out_depth = dst->depth;

        cortex_log_debug("cache hit on game of length %d, eval %f", dst->game.len, dst->eval.evaluation);
        return 1;
    }

    return 0;
}

cortex_eval_cache_entry* _cortex_eval_cache_get_dst(cortex_board* b) {
    //cortex_log_debug("grabbing dst for game:");
    //cortex_move_list_print(&b->move_history);

    /* The cache is based on the moves made in the game. */
    int index = XXH32(b->move_history.list, b->move_history.len * sizeof b->move_history.list[0], 0) % CORTEX_EVAL_CACHE_SIZE;

    //cortex_log_debug("index is %d", index);

    return _cortex_eval_cache + index;
}

void cortex_eval_cache_insert(cortex_board* b, cortex_eval eval, int depth) {
    cortex_eval_cache_entry* dst = _cortex_eval_cache_get_dst(b);

    memcpy(&dst->game, &b->move_history, sizeof dst->game);
    dst->eval = eval;
    dst->depth = depth;
}
