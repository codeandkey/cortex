#pragma once

typedef int cortex_square;

#define CORTEX_SQUARE_NULL -1
#define CORTEX_SQUARE_AT(r, f) ((r-1)*8+(f-1))

cortex_square cortex_square_parse(char* s);
