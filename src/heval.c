#include "heval.h"
#include "log.h"

#include <ctype.h>

float cortex_heval(cortex_position* pos, int verbose) {
    float white_total = 0.0f, black_total = 0.0f;

    float white_material = cortex_heval_material_value(pos, 'w');
    float black_material = cortex_heval_material_value(pos, 'b');

    white_total += white_material;
    black_total += black_material;

    float evaluation = white_total - black_total;

    if (verbose) {
        char fen[CORTEX_POSITION_FEN_BUFLEN];
        cortex_position_write_fen(pos, fen);

        cortex_log("Heuristic evaluation for %s", fen);
        cortex_log("+----------+-------+-------+");
        cortex_log("| Category | White | Black |");
        cortex_log("+----------+-------+-------+");
        cortex_log("| Material | %05.1f | %05.1f |", white_material, black_material);
        cortex_log("+----------+-------+-------+");
        cortex_log("| Total    | %05.1f | %05.1f |", white_total, black_total);
        cortex_log("| Final evaluation: %05.1f  |", evaluation);
        cortex_log("+----------+-------+-------+");
    }

    return evaluation;
}

float cortex_heval_piece_value(char piece) {
    switch (tolower(piece)) {
    case 'k':
        return 0;
    case 'q':
        return 7;
    case 'b':
        return 3;
    case 'r':
        return 5;
    case 'n':
        return 3;
    case 'p':
        return 1;
    default:
        cortex_error("Invalid piece type for heval: %c", piece);
        return -256;
    }
}

float cortex_heval_material_value(cortex_position* pos, char col) {
    float total = 0.0f;

    for (int sq = 0; sq < 64; ++sq) {
        char piece = pos->board[sq];
        if (!piece) continue;

        char lowered = tolower(piece);

        if ((col == 'w') != (lowered != piece)) continue;

        total += cortex_heval_piece_value(lowered);
    }

    return total;
}
