#include "piece.h"

#include <ctype.h>

char cortex_piece_get_color(char piece) {
    return (tolower(piece) == piece) ? 'b' : 'w';
}

char cortex_piece_get_type(char piece) {
    return tolower(piece);
}
