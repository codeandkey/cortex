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
