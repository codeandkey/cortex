#include "eval_cache.h"
#include "xxhash.h"

#include <string.h>

static cortex_eval_cache_entry _cortex_eval_cache[CORTEX_EVAL_CACHE_SIZE];

static cortex_eval_cache_entry* _cortex_eval_cache_get_dst(cortex_board* b);

/*
 * The cache is currently flawed -- board positions with equal states and color to move are considered to be the same position
 * by the hashtable. However, it does not account for double pawn moves/etc altering future legal moves.
 * This will rarely affect the evaluation; but if it does, it could result in huge blunders or even illegal moves
 * suggested by the cached evaluation.
 */
int cortex_eval_try_cache(cortex_board* b, cortex_eval* out) {
    cortex_eval_cache_entry* dst = _cortex_eval_cache_get_dst(b);

    if (dst->filled && !memcmp(dst->state, b->state, sizeof b->state) && b->color_to_move == dst->to_move) {
        *out = dst->eval;
        return 1;
    }

    return 0;
}

cortex_eval_cache_entry* _cortex_eval_cache_get_dst(cortex_board* b) {
    /* First, get the bunch of data needed to hash the position. We include the piece states and color to move. */
    u8 block[65];

    memcpy(block, b->state, sizeof b->state);
    block[64] = b->color_to_move;

    /* Hash the state and search the hashtable. */
    int index = XXH32(block, sizeof block, 0) % CORTEX_EVAL_CACHE_SIZE;

    return _cortex_eval_cache + index;
}

void cortex_eval_cache_insert(cortex_board* b, cortex_eval eval) {
    cortex_eval_cache_entry* dst = _cortex_eval_cache_get_dst(b);

    dst->filled = 1;
    memcpy(dst->state, b->state, sizeof b->state);
    dst->to_move = b->color_to_move;
    dst->eval = eval;
}
