#include "square.h"

#include <ctype.h>

cortex_square cortex_square_parse(char* str) {
    if (str[0] == '-') return CORTEX_SQUARE_NULL;

    char file = tolower(str[0]);
    char rank = str[1];

    if (file > 'h' || file < 'a') return CORTEX_SQUARE_NULL;
    if (rank > '8' || rank < '1') return CORTEX_SQUARE_NULL;

    return CORTEX_SQUARE_AT(rank - '0', file - 'a' + 1);
}

void cortex_square_write(cortex_square sq, char* s) {
    s[0] = cortex_square_file_char(sq);
    s[1] = cortex_square_rank_char(sq);
}

int cortex_square_rank_num(cortex_square sq) {
    if (sq == CORTEX_SQUARE_NULL) return '?';

    return sq / 8 + 1;
}

int cortex_square_file_num(cortex_square sq) {
    if (sq == CORTEX_SQUARE_NULL) return '?';

    return sq % 8 + 1;
}

char cortex_square_rank_char(cortex_square sq) {
    if (sq == CORTEX_SQUARE_NULL) return '?';

    return sq / 8 + '1';
}

char cortex_square_file_char(cortex_square sq) {
    if (sq == CORTEX_SQUARE_NULL) return '?';

    return sq % 8 + 'a';
}

cortex_square cortex_square_offset(cortex_square sq, int ranks, int files) {
    if (sq == CORTEX_SQUARE_NULL) return CORTEX_SQUARE_NULL;

    int f = sq % 8 + 1;
    int r = sq / 8 + 1;

    f += files;
    r += ranks;

    if (f < 1 || f > 8) return CORTEX_SQUARE_NULL;
    if (r < 1 || r > 8) return CORTEX_SQUARE_NULL;

    return CORTEX_SQUARE_AT(r, f);
}
