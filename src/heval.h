#pragma once

/**
 * Methods for evaluating a position at depth 0.
 */

#include "position.h"

/**
 * Gets an immediate evaluation heuristic of a position.
 * Cannot see checks or mates, just directly evaluates the pieces on the board.
 *
 * @param pos Position to evaluate.
 * @return Evaluation value
 */
 float cortex_heval(cortex_position* pos, int verbose);

/**
 * Get the material value belonging to a color.
 * Always non-negative.
 *
 * @param pos Position to evaluate.
 * @param col Color to evaluate: 'w' or 'b'
 * @return Total material value
 */
 float cortex_heval_material_value(cortex_position* pos, char col);

 /**
  * Get the material value of a single piece, regardless of color.
  * Always positive.
  *
  * @param piece Piece.
  * @return Piece value
  */
 float cortex_heval_piece_value(char piece);
