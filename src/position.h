#pragma once

/**
 * Position type. Represents a position in a standard game.
 */

#include "square.h"

#define CORTEX_POSITION_FEN_BUFLEN 128
#define CORTEX_POSITION_FEN_STANDARD "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

typedef struct _cortex_position {
    char board[64];
    int white_castle_kingside, white_castle_queenside;
    int black_castle_kingside, black_castle_queenside;
    char color_to_move; /* 'w' or 'b' */
    cortex_square en_passant_target;
} cortex_position;

/**
 * Loads a standard game.
 *
 * @param dst Position structure to fill
 * @param moves Optional moves to apply from the starting position.
 * @return 0 if successful, error code otherwise
 */
int cortex_position_standard(cortex_position* dst, char* moves);

/**
 * Parses a FEN into a position.
 *
 * @param dst Position structure to fill.
 * @param fen FEN string to parse, along with optional moves to apply
 * @return 0 if successful, error code otherwise
 */
int cortex_position_from_fen(cortex_position* dst, const char* fenmoves);

/**
 * Writes a position FEN to a string.
 *
 * @param pos Position to export
 * @param out Output buffer to fill. Must be of size CORTEX_POSITION_FEN_BUFLEN
 * @return 0 if successful, error code otherwise
 */
int cortex_position_write_fen(cortex_position* pos, char* out);

/**
 * Applies a list of space-separated moves if they are legal.
 *
 * @param dst Position structure to modify.
 * @param moves Space-separated move list.
 * @return 0 if successful, error code otherwise
 */
int cortex_position_apply_moves(cortex_position* pos, char* moves);
