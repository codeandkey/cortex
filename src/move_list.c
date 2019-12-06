#include "move_list.h"

#include <stdio.h>
#include <string.h>

int cortex_move_list_init(cortex_move_list* dst) {
    if (!dst) return -1;
    return (dst->len = 0);
}

void cortex_move_list_add(cortex_move_list* dst, cortex_move m) {
    if (!dst) return;

    dst->list[dst->len++] = m;
}

void cortex_move_list_print(cortex_move_list* dst) {
    if (!dst) return;

    for (int i = 0; i < dst->len; ++i) {
        cortex_move_print_basic(dst->list[i]);
    }
}

int cortex_move_list_contains(cortex_move_list* list, cortex_move mv) {
    if (!list) return -1;

    for (int i = 0; i < list->len; ++i) {
        if (!memcmp(list->list + i, &mv, sizeof mv)) {
            return 1;
        }
    }

    return 0;
}

int cortex_move_list_get(cortex_move_list* dst, cortex_square from, cortex_square to, cortex_move* match) {
    if (!dst || !match) return -1;

    for (int i = 0; i < dst->len; ++i) {
        if (dst->list[i].from == from && dst->list[i].to == to) {
            *match = dst->list[i];
            return 0;
        }
    }

    return -1;
}

int cortex_move_list_equals(cortex_move_list* a, cortex_move_list* b) {
    /* check that one move list is identical to another */
    if (a->len != b->len) return 0;

    for (int i = 0; i < a->len; ++i) {
        if (memcmp(a->list + i, b->list + i, sizeof a->list[0])) return 0;
    }

    return 1;
}
