#pragma once

/***
 * Logging macros.
 *
 * Terminal color output is enabled at compile time.
 * To disable it, do not compile with CORTEX_LOG_COLORS defined.
 *
 */

#include <stdio.h>

#ifdef CORTEX_LOG_COLORS
#define CORTEX_LOG_WHITE "\e[0;39m"
#define CORTEX_LOG_RED "\e[0;31m"
#define CORTEX_LOG_BLUE "\e[0;34m"
#define CORTEX_LOG_CYAN "\e[0;36m"
#define CORTEX_LOG_YELLOW "\e[0;33m"
#define CORTEX_LOG_GREEN "\e[0;32m"
#define CORTEX_LOG_VIOLET "\e[0;35m"
#else
#define CORTEX_LOG_WHITE 
#define CORTEX_LOG_RED
#define CORTEX_LOG_BLUE
#define CORTEX_LOG_CYAN
#define CORTEX_LOG_YELLOW
#define CORTEX_LOG_GREEN
#define CORTEX_LOG_VIOLET
#endif

extern FILE* cortex_log_target;
extern int cortex_debug_enabled;

#define cortex_logf(f, x, ...) fprintf(f, "[%s.%d] " x "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define cortex_log(x, ...) cortex_logf(cortex_log_target, x, ##__VA_ARGS__)
#define cortex_error(x, ...) cortex_logf(cortex_log_target, CORTEX_LOG_RED x CORTEX_LOG_WHITE, ##__VA_ARGS__)
#define cortex_debug(x, ...) if (cortex_debug_enabled) cortex_logf(cortex_log_target, CORTEX_LOG_RED x CORTEX_LOG_WHITE, ##__VA_ARGS__)

int cortex_log_init(FILE* out);
