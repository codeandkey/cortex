#pragma once

/*
 * move list structure
 * used for storing and pruning moves
 */

#include "move.h"

typedef struct _cortex_move_list {
    cortex_move list[1024];
    int len;
} cortex_move_list;

int cortex_move_list_init(cortex_move_list* dst);
void cortex_move_list_add(cortex_move_list* dst, cortex_move sq);

int cortex_move_list_contains(cortex_move_list* dst, cortex_move sq);
void cortex_move_list_print(cortex_move_list* dst);

int cortex_move_list_get(cortex_move_list* dst, cortex_square from, cortex_square to, cortex_move* match);
