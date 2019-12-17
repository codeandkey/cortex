#include "eval.h"
#include "heval.h"
#include "transition.h"
#include "log.h"

#include <pthread.h>
#include <string.h>

static const cortex_eval CORTEX_EVAL_INF = {
    .eval = 0,
    .has_mate = 1,
    .mate = 1,
};

static const cortex_eval CORTEX_EVAL_NEG_INF = {
    .eval = 0,
    .has_mate = 1,
    .mate = -1,
};

static pthread_t _cortex_eval_worker;
static FILE* _cortex_uci_output;
static cortex_position _cortex_eval_root_position;
static int _cortex_eval_nodes;
static int _cortex_eval_running;

static int _cortex_eval_should_stop;
static int _cortex_eval_wtime;
static int _cortex_eval_btime;
static pthread_mutex_t _cortex_eval_should_stop_mut = PTHREAD_MUTEX_INITIALIZER;

static void* _cortex_eval_main(void* a);

static int _cortex_eval_cmp(cortex_eval a, cortex_eval b);
static cortex_eval _cortex_eval_min(cortex_eval a, cortex_eval b);
static cortex_eval _cortex_eval_max(cortex_eval a, cortex_eval b);

static int _cortex_eval_get_should_stop();
static int _cortex_eval_set_should_stop(int should_stop);

static cortex_eval _cortex_eval_alpha_beta(cortex_position* pos, int depth, cortex_eval a, cortex_eval b, char* bestmove);

int cortex_eval_go(cortex_position* pos, FILE* uci_out, int wtime, int btime) {
    if (!pos || !uci_out) return 0;

    /* FIXME: always play e2e4 as white. this stops the game from aborting while the eval is thinking */
    char fen[CORTEX_POSITION_FEN_BUFLEN] = {0};
    cortex_position_write_fen(pos, fen);

    if (!strcmp(fen, CORTEX_POSITION_FEN_STANDARD)) {
        fprintf(uci_out, "bestmove e2e4\n");
        return 0;
    }

    _cortex_uci_output = uci_out;
    memcpy(&_cortex_eval_root_position, pos, sizeof *pos);

    cortex_heval(pos, 1);

    _cortex_eval_running = 1;
    _cortex_eval_wtime = wtime;
    _cortex_eval_btime = btime;

    /* Dispatch worker */
    if (pthread_create(&_cortex_eval_worker, NULL, &_cortex_eval_main, NULL)) {
        cortex_error("Error dispatching eval worker!");
        return -1;
    }

    return 0;
}

void cortex_eval_stop() {
    if (_cortex_eval_running) {
        void* res = NULL;

        if (pthread_join(_cortex_eval_worker, &res)) {
            cortex_error("pthread_join failed!");
        } else {
            cortex_log("Joined evaluation worker with exit code %p", res);
        }

        _cortex_eval_running = 0;
    }
}

void* _cortex_eval_main(void* a) {
    cortex_error("In evaluation worker thread!");

    if (_cortex_eval_set_should_stop(0)) {
        pthread_exit(NULL);
    }

    _cortex_eval_nodes = 0;

    _cortex_eval_get_should_stop(); /* get rid of warnings until UCI STOP is implemented */

    /* Modify the search depth depending on how much time is remaining. */
    int turn_time;

    if (_cortex_eval_root_position.color_to_move == 'w') {
        turn_time = _cortex_eval_wtime;
    } else {
        turn_time = _cortex_eval_btime;
    }

    int current_depth = CORTEX_EVAL_DEPTH;

    /* TODO: these numbers are currently pretty arbitrary. maybe some better numerical analysis could be applied here */
    if (turn_time != -1) {
        if (turn_time < 1000) {
            current_depth = 1;
        } else if (turn_time < 5000) {
            current_depth = 2;
        } else if (turn_time < 10000) {
            current_depth = 3;
        } else if (turn_time < 60000) { 
            current_depth = 4;
        } else {
            current_depth = 5;
        }
    }

    /* Start synchronous alpha beta search */
    char bestmove[6] = {0};
    cortex_eval eval = _cortex_eval_alpha_beta(&_cortex_eval_root_position, current_depth, CORTEX_EVAL_NEG_INF, CORTEX_EVAL_INF, bestmove);

    cortex_log("Finished alpha-beta search with depth %d, %d leaf nodes evaluated", current_depth, _cortex_eval_nodes);

    if (eval.has_mate) {
        cortex_log("Final evaluation: #%d", eval.mate);
    } else {
        cortex_log("Final evaluation: %f", eval.eval);
    }

    fprintf(_cortex_uci_output, "bestmove %s\n", bestmove);

    pthread_exit(0);
    return NULL;
}

int _cortex_eval_get_should_stop() {
    int ret;

    if (pthread_mutex_lock(&_cortex_eval_should_stop_mut)) {
        return -1;
    }

    ret = _cortex_eval_should_stop;

    if (pthread_mutex_unlock(&_cortex_eval_should_stop_mut)) {
        return -1;
    }

    return ret;
}

int _cortex_eval_set_should_stop(int should_stop) {
    if (pthread_mutex_lock(&_cortex_eval_should_stop_mut)) {
        return -1;
    }

    _cortex_eval_should_stop = should_stop;

    if (pthread_mutex_unlock(&_cortex_eval_should_stop_mut)) {
        return -1;
    }

    return 0;
}

int __cortex_eval_cmp(cortex_eval a, cortex_eval b) {
    /*
     * RV:
     * 0 <=> A is equal to B
     * 1 <=> A is greater than B
     * -1 <=> B is greater than A
     */

    if (a.has_mate) {
        if (b.has_mate) {
            /* Both evals have mate. */
            if (a.mate == b.mate) return 0; /* Equal mates */

            if (a.mate > 0) {
                /* A is white mate. A is better as long as B is not a better white mate */
                if (b.mate > 0 && b.mate < a.mate) return -1;
                return 1;
            } else {
                /* A is black mate. B is greater unless it is a better black mate */
                if (b.mate < 0 && b.mate > a.mate) return 1;
                return -1;
            }
        } else {
            /* A has mate but B does not. */
            if (a.mate > 0) return 1;
            if (a.mate < 0) return -1;
        }
    } else {
        if (b.has_mate) {
            /* A does not have mate, but B does. */
            if (b.mate < 0) return 1;
            if (b.mate > 0) return -1;
        } else {
            /* Neither position has mate. */
            if (a.eval > b.eval) return 1;
            if (b.eval > a.eval) return -1;
            return 0;
        }
    }

    cortex_error("_cortex_eval_cmp reached somewhere it shouldn't..");
    return -2;
}

int _cortex_eval_cmp(cortex_eval a, cortex_eval b) {
    return __cortex_eval_cmp(a, b);
    int ret = __cortex_eval_cmp(a, b);
    cortex_log("Comparing a (%f %d? #%d) with b (%f %d? #%d) : result %s", a.eval, a.has_mate, a.mate, b.eval, b.has_mate, b.mate, (ret == 1) ? "A" : (ret == -1 ? "B" : "EQUAL"));
    return ret;
}

cortex_eval _cortex_eval_min(cortex_eval a, cortex_eval b) {
    int c = _cortex_eval_cmp(a, b);

    if (c > 0) return b;
    return a;
}

cortex_eval _cortex_eval_max(cortex_eval a, cortex_eval b) {
    int c = _cortex_eval_cmp(a, b);

    if (c < 0) return b;
    return a;
}

cortex_eval _cortex_eval_alpha_beta(cortex_position* pos, int depth, cortex_eval a, cortex_eval b, char* bestmove) {
    char fen[CORTEX_POSITION_FEN_BUFLEN];
    cortex_position_write_fen(pos, fen);
    cortex_debug("Starting AB search, position %s depth %d", fen, depth);

    if (!depth) {
        cortex_eval out;
        out.has_mate = out.mate = 0;
        out.eval = cortex_heval(pos, 0);
        //char fen[CORTEX_POSITION_FEN_BUFLEN];
        //cortex_position_write_fen(pos, fen);
        //cortex_log("Evaluated position %s at depth 0 : %f", fen, out.eval);
        ++_cortex_eval_nodes;
        return out;
    }

    cortex_eval cur;

    if (pos->color_to_move == 'w') {
        /* Initialize evaluation to #-1 */
        cur = CORTEX_EVAL_NEG_INF;

        /* Generate legal next moves */
        cortex_transition_entry* moves = cortex_transition_list_generate_legal(pos);
        cortex_transition_entry* cur_move = moves;

        cortex_debug("Maximizing evaluation for white's move..");

        /* For each move, run the alphabeta at one lower depth. */
        while (cur_move) {
            cortex_eval ab_result;

            /* Check if the move made will end the game. If it does, no need to evaluate further. */
            int result;
            if (cortex_position_is_game_over(cur_move->transition.result, &result)) {
                if (result) {
                    /* The move is a win! It's impossible for a white move to deliver mate for black */
                    ab_result.has_mate = 1;
                    ab_result.mate = 1;
                    if (bestmove) memcpy(bestmove, cur_move->transition.movestr, sizeof cur_move->transition.movestr);
                    cortex_transition_list_free(moves);
                    return ab_result;
                } else {
                    /* The move delivers a stalemate. Consider it at evaluation 0 */
                    ab_result.has_mate = 0;
                    ab_result.mate = 0;
                    ab_result.eval = 0;
                }
            } else {
                /* Run alpha-beta subsearch, fill in the bestmove as well */
                ab_result = _cortex_eval_alpha_beta(cur_move->transition.result, depth - 1, a, b, NULL);
            }

            cortex_debug("white: Considering move %s => resulting evaluation (%f %d %d)", cur_move->transition.movestr, ab_result.eval, ab_result.has_mate, ab_result.mate);

            int res = _cortex_eval_cmp(ab_result, cur);

            if (res >= 0) {
                /* New best move. */
                if (bestmove) memcpy(bestmove, cur_move->transition.movestr, sizeof cur_move->transition.movestr);
                cur = ab_result;

                cortex_debug("white: Keeping new best move %s", cur_move->transition.movestr);
            }

            a = _cortex_eval_max(a, cur);

            if (_cortex_eval_cmp(a, b) >= 0) {
                cortex_debug("white: Stopping evaluation due alpha-beta pruning");
                break;
            }
            
            cur_move = cur_move->next;
        }

        /* If the eval is a mate, add one move to the dist */
        if (cur.has_mate) {
            if (cur.mate < 0) --cur.mate;
            if (cur.mate > 0) ++cur.mate;
        }

        cortex_transition_list_free(moves);

        return cur;
    } else {
        cortex_debug("Minimizing evaluation for black's move..");

        /* Initialize evaluation to #1 */
        cur = CORTEX_EVAL_INF;

        /* Generate legal next moves */
        cortex_transition_entry* moves = cortex_transition_list_generate_legal(pos);
        cortex_transition_entry* cur_move = moves;

        /* For each move, run the alphabeta at one lower depth. */
        while (cur_move) {
            cortex_eval ab_result;

            /* Check if the move made will end the game. If it does, no need to evaluate further. */
            int result;
            if (cortex_position_is_game_over(cur_move->transition.result, &result)) {
                if (result) {
                    /* The move is a win! It's impossible for a white move to deliver mate for black */
                    ab_result.has_mate = 1;
                    ab_result.mate = -1;
                    if (bestmove) memcpy(bestmove, cur_move->transition.movestr, sizeof cur_move->transition.movestr);
                    cortex_transition_list_free(moves);
                    return ab_result;
                } else {
                    /* The move delivers a stalemate. Consider it at evaluation 0 */
                    ab_result.has_mate = 0;
                    ab_result.mate = 0;
                    ab_result.eval = 0;
                }
            } else {
                /* Run alpha-beta subsearch, fill in the bestmove as well */
                ab_result = _cortex_eval_alpha_beta(cur_move->transition.result, depth - 1, a, b, NULL);
            }

            cortex_debug("black: Considering move %s => resulting evaluation (%f %d %d)", cur_move->transition.movestr, ab_result.eval, ab_result.has_mate, ab_result.mate);

            int res = _cortex_eval_cmp(ab_result, cur);

            if (res <= 0) {
                /* New best move. */
                if (bestmove) memcpy(bestmove, cur_move->transition.movestr, sizeof cur_move->transition.movestr);
                cur = ab_result;

                cortex_debug("black: Keeping new best move %s", cur_move->transition.movestr);
            }

            b = _cortex_eval_min(b, cur);

            if (_cortex_eval_cmp(a, b) >= 0) {
                cortex_debug("black: Stopping evaluation due alpha-beta pruning");
                break;
            }
            
            cur_move = cur_move->next;
        }

        /* If the eval is a mate, add one move to the dist */
        if (cur.has_mate) {
            if (cur.mate < 0) --cur.mate;
            if (cur.mate > 0) ++cur.mate;
        }

        cortex_transition_list_free(moves);

        return cur;
    }
}
