#include "log.h"

#include <time.h>

FILE* cortex_log_target = NULL;

int cortex_log_init(FILE* out) {
    if (!out) return -1;
    cortex_log_target = out;

    time_t current_time;
    time(&current_time);

    cortex_log("Starting log stream. The time is %.24s", ctime(&current_time));
    return 0;
}
