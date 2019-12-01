#pragma once

/*
 * board move type
 *
 * every possible chess move/action is represented in this structure.
 *
 * this includes:
 *   piece move
 *   piece capture
 *   castle (kingside or queenside)
 *   pawn promotion
 *   draw by agreement
 *   resignation
 *
 *  moves will contain attributes, including:
 *    promotion piece (if move is promotion)
 *    move delivering check
 *    move delivering checkmate
 */

#include "types.h"
#include "piece.h"
#include "square.h"

#define CORTEX_MOVE_TYPE_MOVE         0
#define CORTEX_MOVE_TYPE_CAPTURE      1
#define CORTEX_MOVE_TYPE_CASTLE_KING  2
#define CORTEX_MOVE_TYPE_CASTLE_QUEEN 3

#define CORTEX_MOVE_ATTR_NONE    0
#define CORTEX_MOVE_ATTR_CHECK   1
#define CORTEX_MOVE_ATTR_MATE    2
#define CORTEX_MOVE_ATTR_PROMOTE 4

typedef struct _cortex_move {
    u8 move_type;
    u8 move_attr;
    cortex_square from, to; /* only used in MOVE, CAPTURE, and PROMOTE moves */
    cortex_piece_type promote_type; /* only used in PROMOTE moves */
    int complete; /* only set once all of the move fields are filled */
    int is_pawn_double; /* is the move a pawn double move? */
    int is_en_passant;
} cortex_move;

void cortex_move_print_basic(cortex_move m);
