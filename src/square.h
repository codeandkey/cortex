#pragma once

/*
 * board square coordinate system
 */

#include "types.h"

typedef u8 cortex_square;

#define CORTEX_SQUARE_VALID(s) ((s >= 0) && (s < 64))
#define CORTEX_SQUARE_OFFSET(s, ranks, files) (s + ranks * 8 + files)
#define CORTEX_SQUARE_AT(rank, file) CORTEX_SQUARE_OFFSET(0, (rank-1), (file-1))
#define CORTEX_SQUARE_RANK(s) ((s/8)+1)
#define CORTEX_SQUARE_FILE(s) ((s%8)+1)
#define CORTEX_SQUARE_INVALID 255

cortex_square cortex_square_offset(cortex_square s, int ranks, int files);
cortex_square cortex_square_at(int rank, int file);
cortex_square cortex_square_read();
void cortex_square_print(cortex_square s);
