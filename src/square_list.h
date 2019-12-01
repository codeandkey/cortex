#pragma once

/*
 * square list structure
 * used for generating potential moves
 */

#include "square.h"

typedef struct _cortex_square_list {
    cortex_square list[64];
    int len;
} cortex_square_list;

int cortex_square_list_init(cortex_square_list* dst);
void cortex_square_list_add(cortex_square_list* dst, cortex_square sq);

int cortex_square_list_contains(cortex_square_list* dst, cortex_square sq);
void cortex_square_list_print(cortex_square_list* dst);
