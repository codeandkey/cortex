#include "util.h"

#include <ctype.h>

int cortex_util_is_piece(char p) {
    switch (p) {
    case 'k':
    case 'q':
    case 'r':
    case 'b':
    case 'n':
    case 'p':
    case 'K':
    case 'Q':
    case 'R':
    case 'B':
    case 'N':
    case 'P':
        return 1;
    default:
        return 0;
    }
}
