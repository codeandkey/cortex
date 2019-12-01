#include "square_list.h"

#include <stdio.h>

int cortex_square_list_init(cortex_square_list* dst) {
    if (!dst) return -1;
    return (dst->len = 0);
}

void cortex_square_list_add(cortex_square_list* dst, cortex_square sq) {
    if (!dst) return;
    if (cortex_square_list_contains(dst, sq)) return;
    if (!CORTEX_SQUARE_VALID(sq)) return;

    dst->list[dst->len++] = sq;
}

int cortex_square_list_contains(cortex_square_list* dst, cortex_square sq) {
    if (!dst) return 0;

    for (int i = 0; i < dst->len; ++i) {
        if (dst->list[i] == sq) return 1;
    }

    return 0;
}

void cortex_square_list_print(cortex_square_list* dst) {
    if (!dst) return;

    for (int rank = 8; rank >= 1; --rank) {
        for (int file = 1; file <= 8; ++file) {
            if (cortex_square_list_contains(dst, CORTEX_SQUARE_AT(rank, file))) {
                printf("X");
            } else {
                printf(" ");
            }
        }

        printf("\n");
    }
}
