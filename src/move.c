#include "move.h"

#include <stdio.h>

void cortex_move_print_basic(cortex_move m) {
    switch (m.move_type) {
    case CORTEX_MOVE_TYPE_MOVE:
        cortex_square_print(m.from);
        printf(" moves to ");
        cortex_square_print(m.to);
        break;
    case CORTEX_MOVE_TYPE_CAPTURE:
        cortex_square_print(m.from);
        printf(" captures ");
        cortex_square_print(m.to);
        break;
    case CORTEX_MOVE_TYPE_CASTLE_KING:
        printf("kingside castle ");
        break;
    case CORTEX_MOVE_TYPE_CASTLE_QUEEN:
        printf("queenside castle ");
        break;
    }

   if (m.move_attr & CORTEX_MOVE_ATTR_CHECK) {
       printf(" CHECK ");
   }

   if (m.move_attr & CORTEX_MOVE_ATTR_MATE) {
       printf(" MATE ");
   }

   if (m.move_attr & CORTEX_MOVE_ATTR_PROMOTE) {
       printf(" promote=%c ", cortex_piece_type_char(m.promote_type));
   }

   printf("\n");
}
