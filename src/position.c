#include "position.h"
#include "transition.h"
#include "util.h"
#include "log.h"
#include "piece.h"

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

    /* Parse out halfmove clock and fullmove number */
    dst->halfmove_clock = strtol(halfmove_clock, NULL, 10);
    dst->fullmove_number = strtol(fullmove_number, NULL, 10);

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

    /* First, write rank information */
    int opos = 0;

    for (int r = 8; r >= 1; --r) {
        for (int f = 1; f <= 8;) {
            char cur = pos->board[CORTEX_SQUARE_AT(r, f)];

            if (cortex_util_is_piece(cur)) {
                out[opos++] = cur;
                ++f;
            } else {
                /* Walk through all consecutive empty squares */
                int count = 0;

                while (!cortex_util_is_piece(cur) && f <= 8) {
                    ++f;
                    ++count;
                    cur = pos->board[CORTEX_SQUARE_AT(r, f)];
                }

                /* write the count */
                out[opos++] = count + '0';
            }
        }

        if (r > 1) {
            out[opos++] = '/';
        }
    }

    /* Write a space */
    out[opos++] = ' ';

    /* Write active color */
    out[opos++] = pos->color_to_move;

    /* Write a space */
    out[opos++] = ' ';

    /* Write castling availability */
    if (pos->white_castle_kingside) out[opos++] = 'K';
    if (pos->white_castle_queenside) out[opos++] = 'Q';
    if (pos->black_castle_kingside) out[opos++] = 'k';
    if (pos->black_castle_queenside) out[opos++] = 'q';

    /* Write space */
    out[opos++] = ' ';

    /* Write en passant target */
    if (pos->en_passant_target == CORTEX_SQUARE_NULL) {
        out[opos++] = '-';
    } else {
        out[opos++] = cortex_square_file_char(pos->en_passant_target);
        out[opos++] = cortex_square_rank_char(pos->en_passant_target);
    }

    /* Write halfmove clock, fullmove number*/
    opos += snprintf(out + opos, CORTEX_POSITION_FEN_BUFLEN - opos, " %d", pos->halfmove_clock);
    opos += snprintf(out + opos, CORTEX_POSITION_FEN_BUFLEN - opos, " %d", pos->fullmove_number);

    /* null-terminate FEN output */
    out[opos++] = 0;

    return 0;
}

int cortex_position_apply_moves(cortex_position* pos, char* moves) {
    if (!pos) return -1;
    if (!moves) return -2;

    cortex_log("Applying moves %s to position %p", moves, pos);

    char* moves_save;
    for (char* cmove = strtok_r(moves, " \n", &moves_save); cmove; cmove = strtok_r(NULL, " \n", &moves_save)) {
        cortex_log("Applying move %s ...", cmove);

        if (cortex_position_apply_move(pos, cmove)) {
            return -1;
        }
    }

    return 0;
}

int cortex_position_apply_move(cortex_position* pos, char* move) {
    /* temporary UCI protocol hack FIXME */
    if (!strcmp(move, "moves")) return 0;

    /* Grab all legal moves in this position. */
    cortex_transition_entry* legal_moves = cortex_transition_list_generate_legal(pos);

    /* Search the legal moves for the current move */
    cortex_transition_entry* cur = legal_moves, *found = NULL;

    while (cur) {
        if (!strcmp(cur->transition.movestr, move)) {
            found = cur;
            break;
        }

        cur = cur->next;
    }

    if (!found) {
        char fen[CORTEX_POSITION_FEN_BUFLEN];
        cortex_position_write_fen(pos, fen);
        cortex_log("Cannot apply move %s to position %s : illegal move", move, fen);
        return -1;
    }

    /* Apply the found move in-place */
    memcpy(pos, found->transition.result, sizeof *pos);

    legal_moves = cortex_transition_list_free(legal_moves);
    return 0;
}

int cortex_position_get_color_in_check(cortex_position* pos, char color_in_check) {
    cortex_square king = cortex_position_find_king(pos, color_in_check);
    return cortex_position_get_square_in_check(pos, king, color_in_check);
}

int cortex_position_get_square_in_check(cortex_position* pos, cortex_square ksq, char color_in_check) {
    char opponent_color = cortex_util_colorflip(color_in_check);

    for (int sq = 0; sq < 64; ++sq) {
        char piece = pos->board[sq];
        char piece_type = cortex_piece_get_type(piece);
        char piece_color = (piece == piece_type) ? 'b' : 'w';

        int cur_rank = cortex_square_rank_num(sq);
        int cur_file = cortex_square_file_num(sq);

        if (piece_color != opponent_color) continue; /* Wrong turn */

        int pawn_direction = (piece_color == 'w') ? 1 : -1;

        switch (piece_type) {
        case 'k':
            for (int x = -1; x <= 1; ++x) for (int y = -1; y <= 1; ++y) {
                if (!x && !y) continue;

                int target_sq = cortex_square_offset(sq, y, x);
                if (target_sq == ksq) return 1;
            }
            break;
        case 'p':
            /* Check for captures */
            for (int x = -1; x <= 1; x += 2) {
                int target_sq = cortex_square_offset(sq, pawn_direction, x);
                if (target_sq == ksq) return 1;
            }
            break;
        case 'q':
            /* walk right */
            for (int x = cur_file + 1; x <= 8; ++x) {
                cortex_square tsq = CORTEX_SQUARE_AT(cur_rank, x);

                if (tsq == ksq) return 1;
                if (tsq == CORTEX_SQUARE_NULL) break;

                char tpce = pos->board[tsq];

                if (tpce) {
                    break;
                }
            }

            /* walk left */
            for (int x = cur_file - 1; x >= 1; --x) {
                cortex_square tsq = CORTEX_SQUARE_AT(cur_rank, x);

                if (tsq == ksq) return 1;
                if (tsq == CORTEX_SQUARE_NULL) break;

                char tpce = pos->board[tsq];

                if (tpce) {
                    break;
                }
            }

            /* walk up */
            for (int y = cur_rank + 1; y <= 8; ++y) {
                cortex_square tsq = CORTEX_SQUARE_AT(y, cur_file);

                if (tsq == ksq) return 1;
                if (tsq == CORTEX_SQUARE_NULL) break;

                char tpce = pos->board[tsq];

                if (tpce) {
                    break;
                }
            }

            /* walk down */
            for (int y = cur_rank - 1; y >= 1; --y) {
                cortex_square tsq = CORTEX_SQUARE_AT(y, cur_file);

                if (tsq == ksq) return 1;
                if (tsq == CORTEX_SQUARE_NULL) break;

                char tpce = pos->board[tsq];

                if (tpce) {
                    break;
                }
            }

            /* walk up-right */
            for (int i = 1; i <= 7; ++i) {
                cortex_square tsq = cortex_square_offset(sq, i, i);

                if (tsq == CORTEX_SQUARE_NULL) break;
                if (tsq == ksq) return 1;

                char tpce = pos->board[tsq];

                if (tpce) {
                    break;
                }
            }

            /* walk up-left */
            for (int i = 1; i <= 7; ++i) {
                cortex_square tsq = cortex_square_offset(sq, i, -i);

                if (tsq == CORTEX_SQUARE_NULL) break;
                if (tsq == ksq) return 1;

                char tpce = pos->board[tsq];

                if (tpce) {
                    break;
                }
            }

            /* walk down-right */
            for (int i = 1; i <= 7; ++i) {
                cortex_square tsq = cortex_square_offset(sq, -i, i);

                if (tsq == CORTEX_SQUARE_NULL) break;
                if (tsq == ksq) return 1;

                char tpce = pos->board[tsq];

                if (tpce) {
                    break;
                }
            }

            /* walk down-left */
            for (int i = 1; i <= 7; ++i) {
                cortex_square tsq = cortex_square_offset(sq, -i, -i);

                if (tsq == CORTEX_SQUARE_NULL) break;
                if (tsq == ksq) return 1;

                char tpce = pos->board[tsq];

                if (tpce) {
                    break;
                }
            }
            break;
        case 'r':
            /* walk right */
            for (int x = cur_file + 1; x <= 8; ++x) {
                cortex_square tsq = CORTEX_SQUARE_AT(cur_rank, x);

                if (tsq == ksq) return 1;
                if (tsq == CORTEX_SQUARE_NULL) break;

                char tpce = pos->board[tsq];

                if (tpce) {
                    break;
                }
            }

            /* walk left */
            for (int x = cur_file - 1; x >= 1; --x) {
                cortex_square tsq = CORTEX_SQUARE_AT(cur_rank, x);

                if (tsq == ksq) return 1;
                if (tsq == CORTEX_SQUARE_NULL) break;

                char tpce = pos->board[tsq];

                if (tpce) {
                    break;
                }
            }

            /* walk up */
            for (int y = cur_rank + 1; y <= 8; ++y) {
                cortex_square tsq = CORTEX_SQUARE_AT(y, cur_file);

                if (tsq == ksq) return 1;
                if (tsq == CORTEX_SQUARE_NULL) break;

                char tpce = pos->board[tsq];

                if (tpce) {
                    break;
                }
            }

            /* walk down */
            for (int y = cur_rank - 1; y >= 1; --y) {
                cortex_square tsq = CORTEX_SQUARE_AT(y, cur_file);

                if (tsq == ksq) return 1;
                if (tsq == CORTEX_SQUARE_NULL) break;

                char tpce = pos->board[tsq];

                if (tpce) {
                    break;
                }
            }
            break;
        case 'b':
            /* walk up-right */
            for (int i = 1; i <= 7; ++i) {
                cortex_square tsq = cortex_square_offset(sq, i, i);

                if (tsq == CORTEX_SQUARE_NULL) break;
                if (tsq == ksq) return 1;

                char tpce = pos->board[tsq];

                if (tpce) {
                    break;
                }
            }

            /* walk up-left */
            for (int i = 1; i <= 7; ++i) {
                cortex_square tsq = cortex_square_offset(sq, i, -i);

                if (tsq == CORTEX_SQUARE_NULL) break;
                if (tsq == ksq) return 1;

                char tpce = pos->board[tsq];

                if (tpce) {
                    break;
                }
            }

            /* walk down-right */
            for (int i = 1; i <= 7; ++i) {
                cortex_square tsq = cortex_square_offset(sq, -i, i);

                if (tsq == CORTEX_SQUARE_NULL) break;
                if (tsq == ksq) return 1;

                char tpce = pos->board[tsq];

                if (tpce) {
                    break;
                }
            }

            /* walk down-left */
            for (int i = 1; i <= 7; ++i) {
                cortex_square tsq = cortex_square_offset(sq, -i, -i);

                if (tsq == CORTEX_SQUARE_NULL) break;
                if (tsq == ksq) return 1;

                char tpce = pos->board[tsq];

                if (tpce) {
                    break;
                }
            }
            break;
        case 'n':
            for (int a = -1; a <= 1; a += 2) for (int b = -2; b <= 2; b += 4) {
                int tsq = cortex_square_offset(sq, a, b);
                if (tsq == ksq) return 1;

                tsq = cortex_square_offset(sq, b, a);
                if (tsq == ksq) return 1;
            }
            break;
        }
    }

    return 0;
}

cortex_square cortex_position_find_king(cortex_position* pos, char color) {
    int ksq = CORTEX_SQUARE_NULL;
    char kpce = cortex_util_to_color('k', color);

    for (ksq = 0; ksq < 64; ++ksq) {
        if (pos->board[ksq] == kpce) return ksq;
    }

    return CORTEX_SQUARE_NULL;
}
