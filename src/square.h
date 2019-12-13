#pragma once

typedef int cortex_square;

#define CORTEX_SQUARE_NULL -1
#define CORTEX_SQUARE_AT(r, f) ((r-1)*8+(f-1))

cortex_square cortex_square_parse(char* s);
void cortex_square_write(cortex_square sq, char* s);

char cortex_square_rank_char(cortex_square sq);
char cortex_square_file_char(cortex_square sq);
int cortex_square_rank_num(cortex_square sq);
int cortex_square_file_num(cortex_square sq);

cortex_square cortex_square_offset(int sq, int ranks, int files);
