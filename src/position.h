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
    int halfmove_clock;
    int fullmove_number;
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

/**
 * Applies a single move in-place to the position.
 *
 * @param dst Position structure to modify.
 * @param move Move to apply.
 * @return 0 if successful, error code otherwise
 */
int cortex_position_apply_move(cortex_position* pos, char* move);


/**
 * Quickly determines if a square is in check.
 *
 * @param pos Position to examine
 * @param sq Square to check
 * @param color_in_check Color being threatened
 * @return 1 if square is attacked, 0 otherwise
 */
int cortex_position_get_square_in_check(cortex_position* pos, cortex_square sq, char color_in_check);

/**
 * Quickly determine if a color is in check.
 * This uses an early-exit function largely similar to the legal move generation function,
 * except this exits early for better performance.
 *
 * @param pos Position to examine
 * @param color_in_check Check if <color_in_check> is in check
 * @return 1 if <color_in_check> is in check, 0 otherwise
 */
int cortex_position_get_color_in_check(cortex_position* pos, char color_in_check);

/**
 * Finds the king for a certain color.
 *
 * @param pos Position to examine
 * @param color Color to search for
 * @return cortex_square pointing to king, or CORTEX_SQUARE_NULL if not found
 */
cortex_square cortex_position_find_king(cortex_position* pos, char color);

/**
 * Determines if the game is over.
 * Returns 1 if the game is done, 0 if it can continue, or -1 on error.
 * Places the result of the game into *dst if it is non-NULL.
 *
 * @param pos Position
 * @param dst Game state dest
 * @return 1, 0, -1 for game over, game in progress, and error respectively
 */
int cortex_position_is_game_over(cortex_position* pos, int* game_result);
