#include "piece.h"
#include <stdio.h>

char cortex_piece_type_char(cortex_piece p) {
    switch (CORTEX_PIECE_GET_TYPE(p)) {
    case CORTEX_PIECE_TYPE_NONE:
        return ' ';
    case CORTEX_PIECE_TYPE_PAWN:
        return 'b';
    case CORTEX_PIECE_TYPE_KING:
        return 'K';
    case CORTEX_PIECE_TYPE_QUEEN:
        return 'Q';
    case CORTEX_PIECE_TYPE_ROOK:
        return 'R';
    case CORTEX_PIECE_TYPE_BISHOP:
        return 'B';
    case CORTEX_PIECE_TYPE_KNIGHT:
        return 'N';
    default:
        return '?';
    }
}
