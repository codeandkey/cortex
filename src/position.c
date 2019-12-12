#include "position.h"
#include "util.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>

int cortex_position_standard(cortex_position* dst, char* moves) {
    int ret = cortex_position_from_fen(dst, CORTEX_POSITION_FEN_STANDARD);

    if (ret) return ret;
    if (moves) return cortex_position_apply_moves(dst, moves);

    return 0;
}

int cortex_position_from_fen(cortex_position* dst, const char* fen) {
    if (!dst) return -1;
    memset(dst, 0, sizeof *dst);

    char* fenstr = strdup(fen);
    char* fenstr_save;

    /* Grab all six FEN fields and make sure they are there. */
    char* ranks = strtok_r(fenstr, " ", &fenstr_save);
    if (!ranks) return -1;

    char* active_color = strtok_r(NULL, " ", &fenstr_save);
    if (!active_color) return -2;

    char* castle_str = strtok_r(NULL, " ", &fenstr_save);
    if (!castle_str) return -3;

    char* ep_target_str = strtok_r(NULL, " ", &fenstr_save);
    if (!ep_target_str) return -4;

    char* halfmove_clock = strtok_r(NULL, " ", &fenstr_save);
    if (!halfmove_clock) return -5;

    char* fullmove_number = strtok_r(NULL, " \n", &fenstr_save);
    if (!fullmove_number) return -6;

    char* leftover_moves = strtok_r(NULL, " \n", &fenstr_save);

    /* Parse out the board state. */
    char* ranks_save;
    char* cur_rank = NULL;
    for (int r = 8; r >= 1; --r) {
        if (!cur_rank) {
            cur_rank = strtok_r(ranks, "/", &ranks_save);
        } else {
            cur_rank = strtok_r(NULL, "/", &ranks_save);
        }

        if (!cur_rank) return -7;

        /* Walk through the FEN rank and search for pieces or numbers. */
        int cfile = 1;
        for (; *cur_rank; ++cur_rank) {
            if (cortex_util_is_piece(*cur_rank)) {
                dst->board[CORTEX_SQUARE_AT(r, cfile)] = *cur_rank;
                cfile++;
            } else if (*cur_rank >= '1' && *cur_rank <= '8') {
                int count = *cur_rank - '0';

                for (int i = 0; i < count; ++i) {
                    dst->board[CORTEX_SQUARE_AT(r, cfile++)] = 0;
                }
            }
        }
    }

    /* Parse out color to move. */
    dst->color_to_move = *active_color;

    if (dst->color_to_move != 'w' && dst->color_to_move != 'b') return -8;

    /* Parse out castling availabilites */
    for (; *castle_str; ++castle_str) {
        switch (*castle_str) {
        case 'K':
            dst->white_castle_kingside = 1;
            break;
        case 'Q':
            dst->white_castle_queenside = 1;
            break;
        case 'k':
            dst->black_castle_kingside = 1;
            break;
        case 'q':
            dst->black_castle_queenside = 1;
            break;
        default:
            return -9;
        }
    }

    /* Parse out en passant target */
    dst->en_passant_target = cortex_square_parse(ep_target_str);

    /* Apply leftover moves if present */
    if (leftover_moves && strlen(leftover_moves) > 2) {
        int ret = cortex_position_apply_moves(dst, leftover_moves);
        if (ret) return ret;
    }

    /* We could free the temporary fenstr on parse failure but it's OK to just free on success. */
    free(fenstr);

    /* No need to parse the other fields so we do not check them. */
    return 0;
}

int cortex_position_write_fen(cortex_position* pos, char* out) {
    if (!pos) return -1;
    if (!out) return -1;

    /* First, generate board state string */
    char bstate[72]; /* 64 squares, 7 seperators, NULL terminator */
    int bpos = 0;

    for (int r = 8; r >= 1; --r) {
        for (int f = 1; f <= 8;) {
            char cur = pos->board[CORTEX_SQUARE_AT(r, f)];

            if (cortex_util_is_piece(cur)) {
                bstate[bpos++] = cur;
                ++f;
            } else {
                /* Walk through all consecutive empty squares */
                int count = 0;

                while (!cortex_util_is_piece(cur)) {
                    ++f;
                    ++count;
                    cur = pos->board[CORTEX_SQUARE_AT(r, f)];
                }

                /* write the count */
                bstate[bpos++] = count + '0';
            }
        }

        bstate[bpos++] = '/';
    }

    /* null-terminate board state */
    bstate[bpos++] = 0;

    memcpy(out, bstate, strlen(bstate));

    return 0;
}

int cortex_position_apply_moves(cortex_position* pos, char* moves) {
    if (!pos) return -1;
    if (!moves) return -2;

    cortex_log("Applying moves %s to position %p", moves, pos);
    return 0;
}
