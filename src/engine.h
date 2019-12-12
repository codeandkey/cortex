#pragma once

/***
 * Cortex engine bits.
 */

#include <stdio.h>

#include "options.h"
#include "position.h"

#define CORTEX_ENGINE_UCI_BUFLEN 256

typedef struct _cortex_engine {
    cortex_options* opts;
    cortex_position position;
    char input_buf[CORTEX_ENGINE_UCI_BUFLEN];
    int debug;
} cortex_engine;

/**
 * Initializes an engine. This initializes and connects the local evaluation cache.
 * This also initializes an empty internal board.
 * @param eng Engine structure to initialize
 * @param opts Options source
 * @return 0 if successful, error code otherwise.
 */
int cortex_engine_init(cortex_engine* eng, cortex_options* opts);

/**
 * Frees an engine. This closes the cache connection and frees allocated memory.
 * @param eng Engine to free.
 */
void cortex_engine_free(cortex_engine* eng);

/**
 * Starts an engine instance,
 *
 * @param eng Engine to run on.
 *
 * @return 0 if succesful, error code otherwise
 */
int cortex_engine_run(cortex_engine* eng);

/**
 * Immediately reads a line into the UCI buffer.
 * Does not execute the contents of the buffer.
 *
 * @param eng Engine to read command in.
 * @return 0 if read successful, 0 otherwise
 */
int cortex_engine_read_line(cortex_engine* eng);
