#pragma once

/*
 * logging macros
 */

#define cortex_log_debug(x, ...) fprintf(stderr, "%s: " x "\n", __func__, ##__VA_ARGS__)
#define cortex_log_info(x, ...) fprintf(stderr, "%s: " x "\n", __func__, ##__VA_ARGS__)
