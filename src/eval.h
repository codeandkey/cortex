#pragma once

#include "position.h"
#include <stdio.h>

typedef struct _cortex_eval {
    float eval;
    int mate, has_mate;
} cortex_eval;

/**
 * Position evaluation functions.
 * These operate asynchronously and can be interrupted.
 *
 * @param pos Position to evaluate.
 * @param uci_out File handle to output uci 'bestmove' command on.
 *
 * @return 0 if succesfull, error code otherwise
 */
int cortex_eval_go(cortex_position* pos, FILE* uci_out);

/**
 * Stop an evaluation and immediately return the best line.
 */
void cortex_eval_stop();
