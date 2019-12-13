#include "eval.h"
#include "heval.h"
#include "transition.h"
#include "log.h"

int cortex_eval_go(cortex_position* pos, FILE* uci_out) {
    if (!pos || !uci_out) return 0;

    /* TODO: dispatch eval worker thread */
    cortex_heval(pos, 1);

    /* TEMP: generate legal transitions */
    cortex_transition_entry* list = cortex_transition_list_generate_legal(pos);

    if (!list) {
        cortex_error("No legal moves! Cannot output a bestmove.");
        return 0;
    }

    fprintf(uci_out, "info depth 1 seldepth 1 multipv 1 score cp 0 nodes 1 nps 1 tbhits 0 time 1 pv %s\n", list->transition.movestr);
    fprintf(uci_out, "bestmove %s\n", list->transition.movestr);
    return 0;
}

void cortex_eval_stop() {
}
