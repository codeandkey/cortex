#include "square.h"

#include <stdio.h>
#include <ctype.h>

cortex_square cortex_square_offset(cortex_square sq, int ranks, int files) {
    int rank = CORTEX_SQUARE_RANK(sq);
    int file = CORTEX_SQUARE_FILE(sq);

    return cortex_square_at(rank+ranks, file+files);
}

cortex_square cortex_square_at(int rank, int file) {
    if (rank <= 0 || rank > 8) return CORTEX_SQUARE_INVALID;
    if (file <= 0 || file > 8) return CORTEX_SQUARE_INVALID;

    return CORTEX_SQUARE_AT(rank, file);
}

void cortex_square_print(cortex_square sq) {
    int rank = CORTEX_SQUARE_RANK(sq);
    int file = CORTEX_SQUARE_FILE(sq);

    printf("%c%d", (file - 1) + 'a', rank);
}

cortex_square cortex_square_read() {
    int file = 1, rank = 1;

    /* get file */
    while (1) {
        char r = tolower(getchar());
        if (r < 'a' || r > 'h') continue;
        file = r + 1 - 'a';
        break;
    }

    /* get rank */
    while (1) {
        char r = tolower(getchar());
        if (r < '1' || r > '8') continue;
        rank = r + 1 - '1';
        break;
    }

    return cortex_square_at(rank, file);
}
