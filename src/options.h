#pragma once

/**
 * Options structure.
 *
 * This contains all of the engine options and can be modified at runtime.
 * They can also be set at the command line.
 */

#include <stdio.h>

typedef struct _cortex_options {
    int a;
    FILE* in, *out;
} cortex_options;

/**
 * Loads options from the default and the command line.
 *
 * @param dst  Options to fill
 * @param argc Argument count
 * @param argv Argument list
 * @return 0 if successful, error code otherwise
 */
int cortex_options_load(cortex_options* dst, int argc, char** argv);
