#include "options.h"

int cortex_options_load(cortex_options* dst, int argc, char** argv) {
    if (!dst) return -1;

    dst->in = stdin;
    dst->out = stdout;

    return 0;
}
