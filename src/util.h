#pragma once

/**
 * Checks if a character is a piece.
 *
 * @param p Character to test.
 * @return 1 if p is a piece, 0 otherwise
 */
int cortex_util_is_piece(char p);

/**
 * Inverts a color.
 *
 * @param c Color to flip.
 * @return 'b' if (c == 'w'), 'w' otherwise
 */
char cortex_util_colorflip(char c);

/**
 * Converts a piece type to a color.
 *
 * @param t Type
 * @param c Color ('w' or 'b')
 * @return Piece
 */
char cortex_util_to_color(char p, char c);
