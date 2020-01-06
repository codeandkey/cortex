#include "board.h"
#include "log.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int _cortex_board_gen_basic_moves(cortex_board* b, cortex_move_list* out);
static int _cortex_board_gen_basic_moves_for(cortex_board* b, cortex_square sq, cortex_move_list* out);

cortex_piece CORTEX_BOARD_INITIAL_STATE[] = {
    CORTEX_PIECE_WHITE_ROOK, CORTEX_PIECE_WHITE_KNIGHT, CORTEX_PIECE_WHITE_BISHOP, CORTEX_PIECE_WHITE_QUEEN, CORTEX_PIECE_WHITE_KING, CORTEX_PIECE_WHITE_BISHOP, CORTEX_PIECE_WHITE_KNIGHT, CORTEX_PIECE_WHITE_ROOK,
    CORTEX_PIECE_WHITE_PAWN, CORTEX_PIECE_WHITE_PAWN, CORTEX_PIECE_WHITE_PAWN, CORTEX_PIECE_WHITE_PAWN, CORTEX_PIECE_WHITE_PAWN, CORTEX_PIECE_WHITE_PAWN, CORTEX_PIECE_WHITE_PAWN, CORTEX_PIECE_WHITE_PAWN,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    CORTEX_PIECE_BLACK_PAWN, CORTEX_PIECE_BLACK_PAWN, CORTEX_PIECE_BLACK_PAWN, CORTEX_PIECE_BLACK_PAWN, CORTEX_PIECE_BLACK_PAWN, CORTEX_PIECE_BLACK_PAWN, CORTEX_PIECE_BLACK_PAWN, CORTEX_PIECE_BLACK_PAWN,
    CORTEX_PIECE_BLACK_ROOK, CORTEX_PIECE_BLACK_KNIGHT, CORTEX_PIECE_BLACK_BISHOP, CORTEX_PIECE_BLACK_QUEEN, CORTEX_PIECE_BLACK_KING, CORTEX_PIECE_BLACK_BISHOP, CORTEX_PIECE_BLACK_KNIGHT, CORTEX_PIECE_BLACK_ROOK,
};

int cortex_board_init(cortex_board* dst) {
    if (!dst) return -1;

    cortex_log_debug("Initializing blank board state");
    memcpy(dst->state, CORTEX_BOARD_INITIAL_STATE, sizeof dst->state);
    dst->color_to_move = CORTEX_PIECE_COLOR_WHITE;

    cortex_log_debug("Initializing move history");
    cortex_move_list_init(&dst->move_history);

    cortex_log_debug("Generating legal moves");
    cortex_board_gen_legal_moves(dst);

    cortex_log_debug("Initialized standard board at %p", dst);

    return 0;
}

void cortex_board_draw_types(cortex_board* dst) {
    for (int rank = 8; rank >= 1; --rank) {
        for (int file = 1; file <= 8; ++file) {
            fprintf(stderr, "%c", cortex_piece_type_char(dst->state[CORTEX_SQUARE_AT(rank, file)]));
        }

        fprintf(stderr, "\n");
    }
}

int cortex_board_add_attacked_squares(cortex_board* dst, cortex_square sq, cortex_square_list* out) {
    /* get attacked squares from a certain piece */
    if (!dst) return -1;

    cortex_piece from_piece = dst->state[sq];
    cortex_piece_type from_type = CORTEX_PIECE_GET_TYPE(from_piece);
    cortex_piece_color from_color = CORTEX_PIECE_GET_COLOR(from_piece);

    int from_rank = CORTEX_SQUARE_RANK(sq);
    int from_file = CORTEX_SQUARE_FILE(sq);

    int pawn_rank_offset = (from_color == CORTEX_PIECE_COLOR_WHITE) ? 1 : -1;

    switch (from_type) {
    case CORTEX_PIECE_TYPE_NONE:
        return 0;
    case CORTEX_PIECE_TYPE_PAWN:
        cortex_square_list_add(out, cortex_square_offset(sq, pawn_rank_offset, 1));
        cortex_square_list_add(out, cortex_square_offset(sq, pawn_rank_offset, -1));
        return 0;
    case CORTEX_PIECE_TYPE_KING:
        cortex_square_list_add(out, cortex_square_offset(sq, 1, -1));
        cortex_square_list_add(out, cortex_square_offset(sq, 1, 0));
        cortex_square_list_add(out, cortex_square_offset(sq, 1, 1));
        cortex_square_list_add(out, cortex_square_offset(sq, 0, -1));
        cortex_square_list_add(out, cortex_square_offset(sq, 0, 1));
        cortex_square_list_add(out, cortex_square_offset(sq, -1, -1));
        cortex_square_list_add(out, cortex_square_offset(sq, -1, 0));
        cortex_square_list_add(out, cortex_square_offset(sq, -1, 1));
        return 0;
    case CORTEX_PIECE_TYPE_QUEEN:
        for (int x = from_file + 1; x <= 8; ++x) {
            cortex_square_list_add(out, CORTEX_SQUARE_AT(from_rank, x));
            if (dst->state[CORTEX_SQUARE_AT(from_rank, x)]) break;
        }
        for (int x = from_file - 1; x >= 1; --x) {
            cortex_square_list_add(out, CORTEX_SQUARE_AT(from_rank, x));
            if (dst->state[CORTEX_SQUARE_AT(from_rank, x)]) break;
        }
        for (int y = from_rank + 1; y <= 8; ++y) {
            cortex_square_list_add(out, CORTEX_SQUARE_AT(y, from_file));
            if (dst->state[CORTEX_SQUARE_AT(y, from_file)]) break;
        }
        for (int y = from_rank - 1; y >= 1; --y) {
            cortex_square_list_add(out, CORTEX_SQUARE_AT(y, from_file));
            if (dst->state[CORTEX_SQUARE_AT(y, from_file)]) break;
        }
        for (int i = 1; i <= 8; ++i) {
            cortex_square dsq = cortex_square_offset(sq, i, i);
            if (!CORTEX_SQUARE_VALID(dsq)) break;
            cortex_square_list_add(out, dsq);
            if (dst->state[dsq]) break;
        }
        for (int i = 1; i <= 8; ++i) {
            cortex_square dsq = cortex_square_offset(sq, i, -i);
            if (!CORTEX_SQUARE_VALID(dsq)) break;
            cortex_square_list_add(out, dsq);
            if (dst->state[dsq]) break;
        }
        for (int i = 1; i <= 8; ++i) {
            cortex_square dsq = cortex_square_offset(sq, -i, i);
            if (!CORTEX_SQUARE_VALID(dsq)) break;
            cortex_square_list_add(out, dsq);
            if (dst->state[dsq]) break;
        }
        for (int i = 1; i <= 8; ++i) {
            cortex_square dsq = cortex_square_offset(sq, -i, -i);
            if (!CORTEX_SQUARE_VALID(dsq)) break;
            cortex_square_list_add(out, dsq);
            if (dst->state[dsq]) break;
        }
        return 0;
    case CORTEX_PIECE_TYPE_ROOK:
        for (int x = from_file + 1; x <= 8; ++x) {
            cortex_square_list_add(out, CORTEX_SQUARE_AT(from_rank, x));
            if (dst->state[CORTEX_SQUARE_AT(from_rank, x)]) break;
        }
        for (int x = from_file - 1; x >= 1; --x) {
            cortex_square_list_add(out, CORTEX_SQUARE_AT(from_rank, x));
            if (dst->state[CORTEX_SQUARE_AT(from_rank, x)]) break;
        }
        for (int y = from_rank + 1; y <= 8; ++y) {
            cortex_square_list_add(out, CORTEX_SQUARE_AT(y, from_file));
            if (dst->state[CORTEX_SQUARE_AT(y, from_file)]) break;
        }
        for (int y = from_rank - 1; y >= 1; --y) {
            cortex_square_list_add(out, CORTEX_SQUARE_AT(y, from_file));
            if (dst->state[CORTEX_SQUARE_AT(y, from_file)]) break;
        }
        return 0;
    case CORTEX_PIECE_TYPE_BISHOP:
        for (int i = 1; i <= 8; ++i) {
            cortex_square dsq = cortex_square_offset(sq, i, i);
            if (!CORTEX_SQUARE_VALID(dsq)) break;
            cortex_square_list_add(out, dsq);
            if (dst->state[dsq]) break;
        }
        for (int i = 1; i <= 8; ++i) {
            cortex_square dsq = cortex_square_offset(sq, i, -i);
            if (!CORTEX_SQUARE_VALID(dsq)) break;
            cortex_square_list_add(out, dsq);
            if (dst->state[dsq]) break;
        }
        for (int i = 1; i <= 8; ++i) {
            cortex_square dsq = cortex_square_offset(sq, -i, i);
            if (!CORTEX_SQUARE_VALID(dsq)) break;
            cortex_square_list_add(out, dsq);
            if (dst->state[dsq]) break;
        }
        for (int i = 1; i <= 8; ++i) {
            cortex_square dsq = cortex_square_offset(sq, -i, -i);
            if (!CORTEX_SQUARE_VALID(dsq)) break;
            cortex_square_list_add(out, dsq);
            if (dst->state[dsq]) break;
        }
        return 0;
    case CORTEX_PIECE_TYPE_KNIGHT:
        cortex_square_list_add(out, cortex_square_offset(sq, 2, 1));
        cortex_square_list_add(out, cortex_square_offset(sq, 2, -1));
        cortex_square_list_add(out, cortex_square_offset(sq, -2, 1));
        cortex_square_list_add(out, cortex_square_offset(sq, -2, -1));
        cortex_square_list_add(out, cortex_square_offset(sq, 1, 2));
        cortex_square_list_add(out, cortex_square_offset(sq, -1, 2));
        cortex_square_list_add(out, cortex_square_offset(sq, 1, -2));
        cortex_square_list_add(out, cortex_square_offset(sq, -1, -2));
        return 0;
    }

    return -1;
}

int cortex_board_add_attacked_squares_color(cortex_board* dst, cortex_piece_color col, cortex_square_list* out) {
    if (!dst) return -1;

    for (int s = 0; s < 64; ++s) {
        if (CORTEX_PIECE_GET_COLOR(dst->state[s]) != col) continue;
        if (CORTEX_PIECE_GET_TYPE(dst->state[s]) == CORTEX_PIECE_TYPE_NONE) continue;

        if (cortex_board_add_attacked_squares(dst, s, out)) return -1;
    }

    return 0;
}

int cortex_board_get_color_in_check(cortex_board* dst, cortex_piece_color col) {
    if (!dst) return -1;

    /* find the matching king */
    for (int i = 0; i < 64; ++i) {
        cortex_piece p = dst->state[i];

        cortex_piece_type t = CORTEX_PIECE_GET_TYPE(p);
        cortex_piece_color c = CORTEX_PIECE_GET_COLOR(p);

        if (t == CORTEX_PIECE_TYPE_KING && c == col) {
            /* Square located, now invert the color and see if the square is attacked */
            col = !col;

            cortex_square_list sqlist;
            cortex_square_list_init(&sqlist);

            cortex_board_add_attacked_squares_color(dst, col, &sqlist);

            return cortex_square_list_contains(&sqlist, i);
        }
    }

    return -1;
}

int cortex_board_gen_legal_moves(cortex_board* dst) {
    if (!dst) return -1;

    cortex_move_list_init(&dst->legal_moves);

    /* Clear the move list, we will generate it from scratch. */
    cortex_move_list basic_moves;
    cortex_move_list_init(&basic_moves);

    /* Generate basic moves. */
    _cortex_board_gen_basic_moves(dst, &basic_moves);

    cortex_move_list pruned_basic_moves;
    cortex_move_list_init(&pruned_basic_moves);

    /* From basic moves, prune moves that place the moving color into check. */
    for (int i = 0; i < basic_moves.len; ++i) {
        if (!cortex_board_complete_move(dst, basic_moves.list + i, NULL)) {
            cortex_move_list_add(&dst->legal_moves, basic_moves.list[i]);
        }
    }

    return 0;
}

int _cortex_board_gen_basic_moves(cortex_board* b, cortex_move_list* out) {
    if (!b || !out) return -1;

    /* Basic moves include moves, captures, and promotions. */
    for (int sq = 0; sq < 64; ++sq) {
        if (CORTEX_PIECE_GET_COLOR(b->state[sq]) == b->color_to_move) {
            if (_cortex_board_gen_basic_moves_for(b, sq, out)) return -1;
        }
    }

    return 0;
}

int _cortex_board_gen_basic_moves_for(cortex_board* b, cortex_square sq, cortex_move_list* out) {
    if (!b || !out) return -1;

    cortex_piece from_piece = b->state[sq];
    cortex_piece_type from_type = CORTEX_PIECE_GET_TYPE(from_piece);
    cortex_piece_color from_color = CORTEX_PIECE_GET_COLOR(from_piece);

    int from_rank = CORTEX_SQUARE_RANK(sq);
    int from_file = CORTEX_SQUARE_FILE(sq);

    int pawn_rank_offset = (from_color == CORTEX_PIECE_COLOR_WHITE) ? 1 : -1;
    int initial_pawn_rank = (from_color == CORTEX_PIECE_COLOR_WHITE) ? 2 : 7;
    int last_pawn_rank = initial_pawn_rank + pawn_rank_offset * 6;

    cortex_move tmp_move;
    tmp_move.complete = 0; /* all of the moves generated here are incomplete (missing attrs) */
    tmp_move.move_attr = CORTEX_MOVE_ATTR_NONE;
    tmp_move.promote_type = CORTEX_PIECE_TYPE_NONE;
    tmp_move.is_pawn_double = 0;
    tmp_move.is_en_passant = 0;

    switch (from_type) {
    case CORTEX_PIECE_TYPE_NONE:
        return 0;
    case CORTEX_PIECE_TYPE_PAWN:
        {
            cortex_square pawn_move_target = cortex_square_offset(sq, pawn_rank_offset, 0);

            if (pawn_move_target != CORTEX_SQUARE_INVALID && !b->state[pawn_move_target]) {
                tmp_move.from = sq;
                tmp_move.to = pawn_move_target;
                tmp_move.move_type = CORTEX_MOVE_TYPE_MOVE;

                /* if the move is a potential promotion, then add all 4 promotions */
                if (CORTEX_SQUARE_RANK(pawn_move_target) == last_pawn_rank) {
                    tmp_move.move_attr |= CORTEX_MOVE_ATTR_PROMOTE;

                    tmp_move.promote_type = CORTEX_PIECE_TYPE_QUEEN;
                    cortex_move_list_add(out, tmp_move);

                    tmp_move.promote_type = CORTEX_PIECE_TYPE_KNIGHT;
                    cortex_move_list_add(out, tmp_move);

                    tmp_move.promote_type = CORTEX_PIECE_TYPE_ROOK;
                    cortex_move_list_add(out, tmp_move);

                    tmp_move.promote_type = CORTEX_PIECE_TYPE_BISHOP;
                    cortex_move_list_add(out, tmp_move);

                    tmp_move.move_attr = 0;
                } else {
                    cortex_move_list_add(out, tmp_move);
                }
            }

            /* if the pawn is on the initial pawn rank, double move if legal */
            if (from_rank == initial_pawn_rank && !b->state[pawn_move_target]) {
                pawn_move_target = cortex_square_offset(pawn_move_target, pawn_rank_offset, 0);

                if (!b->state[pawn_move_target]) {
                    tmp_move.to = pawn_move_target;
                    tmp_move.is_pawn_double = 1;
                    cortex_move_list_add(out, tmp_move);
                    tmp_move.is_pawn_double = 0;
                }
            }

            /* add legal capture moves */
            for (int dir = -1; dir <= 1; dir += 2) {
                cortex_square pawn_capture_target = cortex_square_offset(sq, pawn_rank_offset, dir);

                if (pawn_capture_target == CORTEX_SQUARE_INVALID) continue;
                cortex_piece target_piece = b->state[pawn_capture_target];

                if (CORTEX_PIECE_GET_COLOR(target_piece) != from_color && CORTEX_PIECE_GET_TYPE(target_piece) != CORTEX_PIECE_TYPE_NONE) {
                    /* Valid enemy piece capture. */
                    tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                    tmp_move.from = sq;
                    tmp_move.to = pawn_capture_target;
                    cortex_move_list_add(out, tmp_move);
                }

                /* If the last move was a pawn double move, consider en passant captures */
                if (b->move_history.len > 0) {
                    cortex_move last_move = b->move_history.list[b->move_history.len - 1];

                    if (last_move.is_pawn_double) {
                        if (pawn_capture_target == cortex_square_offset(last_move.to, pawn_rank_offset, 0)) {
                            /* this pawn can be captured en passant. */
                            tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                            tmp_move.from = sq;
                            tmp_move.to = pawn_capture_target;
                            tmp_move.is_en_passant = 1;
                            cortex_move_list_add(out, tmp_move);
                            tmp_move.is_en_passant = 0;
                        }
                    }
                }
            }
        }
        return 0;
    case CORTEX_PIECE_TYPE_KING:
        /* walk in a circle around the king. */
        for (int x = -1; x <= 1; ++x) {
            for (int y = -1;  y <= 1; ++y) {
                if (!x && !y) continue;

                cortex_square king_target = cortex_square_offset(sq, x, y);

                if (king_target == CORTEX_SQUARE_INVALID) continue;
                cortex_piece dest_piece = b->state[king_target];

                tmp_move.from = sq;
                tmp_move.to = king_target;

                if (dest_piece) {
                    if (CORTEX_PIECE_GET_COLOR(dest_piece) != from_color) {
                        /* king can capture */
                        tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                        cortex_move_list_add(out, tmp_move);
                    }
                } else {
                    /* open square, king can move */
                    tmp_move.move_type = CORTEX_MOVE_TYPE_MOVE;
                    cortex_move_list_add(out, tmp_move);
                }
            }
        }
        return 0;
    case CORTEX_PIECE_TYPE_QUEEN:
        {
            cortex_square queen_target;

            tmp_move.from = sq;

            /* walk right */
            for (int x = from_file + 1; x <= 8; ++x) {
                queen_target = CORTEX_SQUARE_AT(from_rank, x);

                if (b->state[queen_target]) {
                    if (CORTEX_PIECE_GET_COLOR(b->state[queen_target]) != from_color) {
                        tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                        tmp_move.to = queen_target;
                        cortex_move_list_add(out, tmp_move);
                    }
                    break;
                } else {
                    tmp_move.move_type = CORTEX_MOVE_TYPE_MOVE;
                    tmp_move.to = queen_target;
                    cortex_move_list_add(out, tmp_move);
                }
            }

            /* walk left */
            for (int x = from_file - 1; x >= 1; --x) {
                queen_target = CORTEX_SQUARE_AT(from_rank, x);

                if (b->state[queen_target]) {
                    if (CORTEX_PIECE_GET_COLOR(b->state[queen_target]) != from_color) {
                        tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                        tmp_move.to = queen_target;
                        cortex_move_list_add(out, tmp_move);
                    }
                    break;
                } else {
                    tmp_move.move_type = CORTEX_MOVE_TYPE_MOVE;
                    tmp_move.to = queen_target;
                    cortex_move_list_add(out, tmp_move);
                }
            }

            /* walk up */
            for (int y = from_rank + 1; y <= 8; ++y) {
                queen_target = CORTEX_SQUARE_AT(y, from_file);

                if (b->state[queen_target]) {
                    if (CORTEX_PIECE_GET_COLOR(b->state[queen_target]) != from_color) {
                        tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                        tmp_move.to = queen_target;
                        cortex_move_list_add(out, tmp_move);
                    }
                    break;
                } else {
                    tmp_move.move_type = CORTEX_MOVE_TYPE_MOVE;
                    tmp_move.to = queen_target;
                    cortex_move_list_add(out, tmp_move);
                }
            }

            /* walk down */
            for (int y = from_rank - 1; y >= 1; --y) {
                queen_target = CORTEX_SQUARE_AT(y, from_file);

                if (b->state[queen_target]) {
                    if (CORTEX_PIECE_GET_COLOR(b->state[queen_target]) != from_color) {
                        tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                        tmp_move.to = queen_target;
                        cortex_move_list_add(out, tmp_move);
                    }
                    break;
                } else {
                    tmp_move.move_type = CORTEX_MOVE_TYPE_MOVE;
                    tmp_move.to = queen_target;
                    cortex_move_list_add(out, tmp_move);
                }
            }

            /* walk up-right */
            for (int i = 1; i <= 8; ++i) {
                queen_target = cortex_square_offset(sq, i, i);

                if (queen_target == CORTEX_SQUARE_INVALID) break;

                if (b->state[queen_target]) {
                    if (CORTEX_PIECE_GET_COLOR(b->state[queen_target]) != from_color) {
                        tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                        tmp_move.to = queen_target;
                        cortex_move_list_add(out, tmp_move);
                    }
                    break;
                } else {
                    tmp_move.move_type = CORTEX_MOVE_TYPE_MOVE;
                    tmp_move.to = queen_target;
                    cortex_move_list_add(out, tmp_move);
                }
            }

            /* walk up-left */
            for (int i = 1; i <= 8; ++i) {
                queen_target = cortex_square_offset(sq, i, -i);

                if (queen_target == CORTEX_SQUARE_INVALID) break;

                if (b->state[queen_target]) {
                    if (CORTEX_PIECE_GET_COLOR(b->state[queen_target]) != from_color) {
                        tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                        tmp_move.to = queen_target;
                        cortex_move_list_add(out, tmp_move);
                    }
                    break;
                } else {
                    tmp_move.move_type = CORTEX_MOVE_TYPE_MOVE;
                    tmp_move.to = queen_target;
                    cortex_move_list_add(out, tmp_move);
                }
            }

            /* walk down-left */
            for (int i = 1; i <= 8; ++i) {
                queen_target = cortex_square_offset(sq, -i, -i);

                if (queen_target == CORTEX_SQUARE_INVALID) break;

                if (b->state[queen_target]) {
                    if (CORTEX_PIECE_GET_COLOR(b->state[queen_target]) != from_color) {
                        tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                        tmp_move.to = queen_target;
                        cortex_move_list_add(out, tmp_move);
                    }
                    break;
                } else {
                    tmp_move.move_type = CORTEX_MOVE_TYPE_MOVE;
                    tmp_move.to = queen_target;
                    cortex_move_list_add(out, tmp_move);
                }
            }

            /* walk down-right */
            for (int i = 1; i <= 8; ++i) {
                queen_target = cortex_square_offset(sq, -i, i);

                if (queen_target == CORTEX_SQUARE_INVALID) break;

                if (b->state[queen_target]) {
                    if (CORTEX_PIECE_GET_COLOR(b->state[queen_target]) != from_color) {
                        tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                        tmp_move.to = queen_target;
                        cortex_move_list_add(out, tmp_move);
                    }
                    break;
                } else {
                    tmp_move.move_type = CORTEX_MOVE_TYPE_MOVE;
                    tmp_move.to = queen_target;
                    cortex_move_list_add(out, tmp_move);
                }
            }
        }
        return 0;
    case CORTEX_PIECE_TYPE_ROOK:
        { /* just borrow queen logic */
            cortex_square queen_target;

            tmp_move.from = sq;

            /* walk right */
            for (int x = from_file + 1; x <= 8; ++x) {
                queen_target = CORTEX_SQUARE_AT(from_rank, x);

                if (b->state[queen_target]) {
                    if (CORTEX_PIECE_GET_COLOR(b->state[queen_target]) != from_color) {
                        tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                        tmp_move.to = queen_target;
                        cortex_move_list_add(out, tmp_move);
                    }
                    break;
                } else {
                    tmp_move.move_type = CORTEX_MOVE_TYPE_MOVE;
                    tmp_move.to = queen_target;
                    cortex_move_list_add(out, tmp_move);
                }
            }

            /* walk left */
            for (int x = from_file - 1; x >= 1; --x) {
                queen_target = CORTEX_SQUARE_AT(from_rank, x);

                if (b->state[queen_target]) {
                    if (CORTEX_PIECE_GET_COLOR(b->state[queen_target]) != from_color) {
                        tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                        tmp_move.to = queen_target;
                        cortex_move_list_add(out, tmp_move);
                    }
                    break;
                } else {
                    tmp_move.move_type = CORTEX_MOVE_TYPE_MOVE;
                    tmp_move.to = queen_target;
                    cortex_move_list_add(out, tmp_move);
                }
            }

            /* walk up */
            for (int y = from_rank + 1; y <= 8; ++y) {
                queen_target = CORTEX_SQUARE_AT(y, from_file);

                if (b->state[queen_target]) {
                    if (CORTEX_PIECE_GET_COLOR(b->state[queen_target]) != from_color) {
                        tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                        tmp_move.to = queen_target;
                        cortex_move_list_add(out, tmp_move);
                    }
                    break;
                } else {
                    tmp_move.move_type = CORTEX_MOVE_TYPE_MOVE;
                    tmp_move.to = queen_target;
                    cortex_move_list_add(out, tmp_move);
                }
            }

            /* walk down */
            for (int y = from_rank - 1; y >= 1; --y) {
                queen_target = CORTEX_SQUARE_AT(y, from_file);

                if (b->state[queen_target]) {
                    if (CORTEX_PIECE_GET_COLOR(b->state[queen_target]) != from_color) {
                        tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                        tmp_move.to = queen_target;
                        cortex_move_list_add(out, tmp_move);
                    }
                    break;
                } else {
                    tmp_move.move_type = CORTEX_MOVE_TYPE_MOVE;
                    tmp_move.to = queen_target;
                    cortex_move_list_add(out, tmp_move);
                }
            }
        }
        return 0;
    case CORTEX_PIECE_TYPE_BISHOP:
        {
            cortex_square queen_target;

            tmp_move.from = sq;

            /* walk up-right */
            for (int i = 1; i <= 8; ++i) {
                queen_target = cortex_square_offset(sq, i, i);

                if (queen_target == CORTEX_SQUARE_INVALID) break;

                if (b->state[queen_target]) {
                    if (CORTEX_PIECE_GET_COLOR(b->state[queen_target]) != from_color) {
                        tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                        tmp_move.to = queen_target;
                        cortex_move_list_add(out, tmp_move);
                    }
                    break;
                } else {
                    tmp_move.move_type = CORTEX_MOVE_TYPE_MOVE;
                    tmp_move.to = queen_target;
                    cortex_move_list_add(out, tmp_move);
                }
            }

            /* walk up-left */
            for (int i = 1; i <= 8; ++i) {
                queen_target = cortex_square_offset(sq, i, -i);

                if (queen_target == CORTEX_SQUARE_INVALID) break;

                if (b->state[queen_target]) {
                    if (CORTEX_PIECE_GET_COLOR(b->state[queen_target]) != from_color) {
                        tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                        tmp_move.to = queen_target;
                        cortex_move_list_add(out, tmp_move);
                    }
                    break;
                } else {
                    tmp_move.move_type = CORTEX_MOVE_TYPE_MOVE;
                    tmp_move.to = queen_target;
                    cortex_move_list_add(out, tmp_move);
                }
            }

            /* walk down-left */
            for (int i = 1; i <= 8; ++i) {
                queen_target = cortex_square_offset(sq, -i, -i);

                if (queen_target == CORTEX_SQUARE_INVALID) break;

                if (b->state[queen_target]) {
                    if (CORTEX_PIECE_GET_COLOR(b->state[queen_target]) != from_color) {
                        tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                        tmp_move.to = queen_target;
                        cortex_move_list_add(out, tmp_move);
                    }
                    break;
                } else {
                    tmp_move.move_type = CORTEX_MOVE_TYPE_MOVE;
                    tmp_move.to = queen_target;
                    cortex_move_list_add(out, tmp_move);
                }
            }

            /* walk down-right */
            for (int i = 1; i <= 8; ++i) {
                queen_target = cortex_square_offset(sq, -i, i);

                if (queen_target == CORTEX_SQUARE_INVALID) break;

                if (b->state[queen_target]) {
                    if (CORTEX_PIECE_GET_COLOR(b->state[queen_target]) != from_color) {
                        tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                        tmp_move.to = queen_target;
                        cortex_move_list_add(out, tmp_move);
                    }
                    break;
                } else {
                    tmp_move.move_type = CORTEX_MOVE_TYPE_MOVE;
                    tmp_move.to = queen_target;
                    cortex_move_list_add(out, tmp_move);
                }
            }
        }
        return 0;
    case CORTEX_PIECE_TYPE_KNIGHT:
        for (int small = -1; small <= 1; small += 2) {
            for (int large = -2; large <= 2; large += 4) {
                cortex_square knight_target = cortex_square_offset(sq, small, large);

                if (knight_target != CORTEX_SQUARE_INVALID) {
                    if (b->state[knight_target]) {
                        if (CORTEX_PIECE_GET_COLOR(b->state[knight_target]) != from_color) {
                            /* valid knight capture */
                            tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                            tmp_move.from = sq;
                            tmp_move.to = knight_target;
                            cortex_move_list_add(out, tmp_move);
                        }
                    } else {
                        /* valid knight move */
                        tmp_move.move_type = CORTEX_MOVE_TYPE_MOVE;
                        tmp_move.from = sq;
                        tmp_move.to = knight_target;
                        cortex_move_list_add(out, tmp_move);
                    }
                }

                /* then, try the other combination */
                knight_target = cortex_square_offset(sq, large, small);

                if (knight_target != CORTEX_SQUARE_INVALID) {
                    if (b->state[knight_target]) {
                        if (CORTEX_PIECE_GET_COLOR(b->state[knight_target]) != from_color) {
                            /* valid knight capture */
                            tmp_move.move_type = CORTEX_MOVE_TYPE_CAPTURE;
                            tmp_move.from = sq;
                            tmp_move.to = knight_target;
                            cortex_move_list_add(out, tmp_move);
                        }
                    } else {
                        /* valid knight move */
                        tmp_move.move_type = CORTEX_MOVE_TYPE_MOVE;
                        tmp_move.from = sq;
                        tmp_move.to = knight_target;
                        cortex_move_list_add(out, tmp_move);
                    }
                }
            }
        }
        return 0;
    }

    return -1;
}

int cortex_board_complete_move(cortex_board* dst, cortex_move* move, cortex_board* out) {
    if (!dst) return -1;

    /* apply a basic move, without checking for illegal states */
    /* this includes moves, captures, and promotions */

    cortex_board result;

    /* Copy over board state */
    memcpy(&result, dst, sizeof result);

    /* Modify resulting state */
    result.state[move->to] = result.state[move->from];
    result.state[move->from] = 0;

    /* If move is a promotion, modify the piece type */
    if (move->move_attr & CORTEX_MOVE_ATTR_PROMOTE) {
        result.state[move->to] = move->promote_type;

        if (result.color_to_move == CORTEX_PIECE_COLOR_WHITE) {
            result.state[move->to] = CORTEX_PIECE_TO_WHITE(result.state[move->to]);
        }
    }

    /* If move is en-passant, find and remove the pawn captured */
    if (move->is_en_passant) {
        result.state[result.move_history.list[result.move_history.len - 1].to] = 0;
    }

    /* If the color that just moved is in check, the move is illegal. */
    if (cortex_board_get_color_in_check(&result, result.color_to_move)) {
        return -1;
    }

    cortex_move_list_add(&result.move_history, *move);
    result.color_to_move = !result.color_to_move;

    /* If the other color is in check, add a check attr. */
    if (cortex_board_get_color_in_check(&result, result.color_to_move)) {
        cortex_board_gen_legal_moves(&result);

        if (!result.legal_moves.len) {
            /* No legal moves for the other color. Mark the move as mate instead of check. */
            result.move_history.list[result.move_history.len - 1].move_attr |= CORTEX_MOVE_ATTR_MATE;
            move->move_attr |= CORTEX_MOVE_ATTR_MATE;
        } else {
            result.move_history.list[result.move_history.len - 1].move_attr |= CORTEX_MOVE_ATTR_CHECK;
            move->move_attr |= CORTEX_MOVE_ATTR_CHECK;
        }
    }

    move->complete = 1;

    /* Copy the new state if needed. */
    if (out) {
        memcpy(out, &result, sizeof result);
    }

    return 0;
}

int cortex_board_apply_move(cortex_board* dst, cortex_move move) {
    if (!dst) return -1;

    /* Check that the move is legal. */
    if (!cortex_move_list_contains(&dst->legal_moves, move)) return -1;

    /* Apply the move and grab the new state */
    cortex_board_complete_move(dst, &move, dst);

    cortex_board_gen_legal_moves(dst);

    return 0;
}
