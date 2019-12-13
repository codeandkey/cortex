#include "transition.h"
#include "util.h"
#include "piece.h"

#include <stdlib.h>
#include <string.h>

static cortex_transition_entry* _cortex_transition_list_generate_basic(cortex_position* pos);
static cortex_transition_entry* _cortex_transition_list_prune_in_check(cortex_transition_entry* head, char color_in_check);

static cortex_transition_entry* _cortex_transition_list_append_simple_move(cortex_transition_entry* head, cortex_position* pos, cortex_square from, cortex_square to, char promote);
static cortex_transition_entry* _cortex_transition_list_append_castles(cortex_transition_entry* head, cortex_position* pos);

cortex_transition_entry* cortex_transition_list_generate_legal(cortex_position* pos) {
    if (!pos) return NULL;

    cortex_transition_entry* list = _cortex_transition_list_generate_basic(pos);

    list = _cortex_transition_list_append_castles(list, pos);

    /* prune out moves that result in the color that moved being in check */
    list = _cortex_transition_list_prune_in_check(list, pos->color_to_move);

    return list;
}

cortex_transition_entry* _cortex_transition_list_generate_basic(cortex_position* pos) {
    /*
     * Basic moves include moves, captures, promotions, and castles.
     * This generation does not account for illegal moves via checks.
     */

    cortex_transition_entry* output_head = NULL;

    for (int sq = 0; sq < 64; ++sq) {
        char piece = pos->board[sq];
        char piece_type = cortex_piece_get_type(piece);
        char piece_color = (piece == piece_type) ? 'b' : 'w';

        int cur_rank = cortex_square_rank_num(sq);
        int cur_file = cortex_square_file_num(sq);

        if (piece_color != pos->color_to_move) continue; /* Wrong turn */

        int pawn_direction = (piece_color == 'w') ? 1 : -1;
        int pawn_first_rank = (piece_color == 'w') ? 2 : 7;
        int pawn_last_rank = (piece_color == 'w') ? 8 : 1;
        int pawn_ep_target_rank = (piece_color == 'w') ? 6 : 3;

        switch (piece_type) {
        case 'k':
            for (int x = -1; x <= 1; ++x) for (int y = -1; y <= 1; ++y) {
                if (!x && !y) continue;

                int target_sq = cortex_square_offset(sq, y, x);
                if (target_sq == CORTEX_SQUARE_NULL) continue;

                char target_piece = pos->board[target_sq];

                if (target_piece) {
                    char target_piece_color = cortex_piece_get_color(target_piece);

                    if (target_piece_color != pos->color_to_move) {
                        /* Potential king capture */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, target_sq, 0);
                    }
                } else {
                    /* Potential king move */
                    output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, target_sq, 0);
                }
            }
            break;
        case 'p':
            /* Somehow pawns probably have the most complicated movement rules in chess. */

            /* Check for captures */
            for (int x = -1; x <= 1; x += 2) {
                int target_sq = cortex_square_offset(sq, pawn_direction, x);

                if (target_sq == CORTEX_SQUARE_NULL) continue;

                char target_piece = pos->board[target_sq];

                /* Is the target an enemy piece? Normal capture, or promotion */
                if (target_piece || (target_sq == pos->en_passant_target && cur_rank + pawn_direction == pawn_ep_target_rank)) {
                    if (cortex_piece_get_color(target_piece) != pos->color_to_move) {
                        /* Potential capture, check if promotion */
                        if (cur_rank + pawn_direction == pawn_last_rank) {
                            /* Promoting capture */
                            output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, target_sq, 'q');
                            output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, target_sq, 'b');
                            output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, target_sq, 'n');
                            output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, target_sq, 'r');
                        } else {
                            /* Non-promotion */
                            output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, target_sq, 0);
                        }
                    }
                }
            }

            /* Pawn advance tests */
            {
                int first_sq = cortex_square_offset(sq, pawn_direction, 0);
                char first_pce = pos->board[first_sq];

                if (!first_pce) {
                    /* Potential pawn advance (single) */
                    /* Check if promotion */
                    if (cur_rank + pawn_direction == pawn_last_rank) {
                        /* Promoting capture */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, first_sq, 'q');
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, first_sq, 'b');
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, first_sq, 'n');
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, first_sq, 'r');
                    } else {
                        /* Non-promotion */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, first_sq, 0);
                    }

                    /* Square is empty. If on the first rank, check for pawn jumps */
                    if (cur_rank == pawn_first_rank) {
                        int second_sq = cortex_square_offset(first_sq, pawn_direction, 0);
                        char second_pce = pos->board[second_sq];

                        if (!second_pce) {
                            /* Non-promotion is implied here */
                            output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, second_sq, 0);
                        } 
                    }
                }
            }
            break;
        case 'q':
            /* walk right */
            for (int x = cur_file + 1; x <= 8; ++x) {
                cortex_square tsq = CORTEX_SQUARE_AT(cur_rank, x);
                char tpce = pos->board[tsq];

                if (tpce) {
                    if (cortex_piece_get_color(tpce) != pos->color_to_move) {
                        /* Potential capture */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                    }

                    break;
                } else {
                    /* Potential move */
                    output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                }
            }

            /* walk left */
            for (int x = cur_file - 1; x >= 1; --x) {
                cortex_square tsq = CORTEX_SQUARE_AT(cur_rank, x);
                char tpce = pos->board[tsq];

                if (tpce) {
                    if (cortex_piece_get_color(tpce) != pos->color_to_move) {
                        /* Potential capture */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                    }

                    break;
                } else {
                    /* Potential move */
                    output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                }
            }

            /* walk up */
            for (int y = cur_rank + 1; y <= 8; ++y) {
                cortex_square tsq = CORTEX_SQUARE_AT(y, cur_file);
                char tpce = pos->board[tsq];

                if (tpce) {
                    if (cortex_piece_get_color(tpce) != pos->color_to_move) {
                        /* Potential capture */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                    }

                    break;
                } else {
                    /* Potential move */
                    output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                }
            }

            /* walk down */
            for (int y = cur_rank - 1; y >= 1; --y) {
                cortex_square tsq = CORTEX_SQUARE_AT(y, cur_file);
                char tpce = pos->board[tsq];

                if (tpce) {
                    if (cortex_piece_get_color(tpce) != pos->color_to_move) {
                        /* Potential capture */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                    }

                    break;
                } else {
                    /* Potential move */
                    output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                }
            }

            /* walk up-right */
            for (int i = 1; i <= 7; ++i) {
                cortex_square tsq = cortex_square_offset(sq, i, i);

                if (tsq == CORTEX_SQUARE_NULL) break;

                char tpce = pos->board[tsq];

                if (tpce) {
                    if (cortex_piece_get_color(tpce) != pos->color_to_move) {
                        /* Potential capture */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                    }

                    break;
                } else {
                    /* Potential move */
                    output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                }
            }

            /* walk up-left */
            for (int i = 1; i <= 7; ++i) {
                cortex_square tsq = cortex_square_offset(sq, i, -i);

                if (tsq == CORTEX_SQUARE_NULL) break;

                char tpce = pos->board[tsq];

                if (tpce) {
                    if (cortex_piece_get_color(tpce) != pos->color_to_move) {
                        /* Potential capture */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                    }

                    break;
                } else {
                    /* Potential move */
                    output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                }
            }

            /* walk down-right */
            for (int i = 1; i <= 7; ++i) {
                cortex_square tsq = cortex_square_offset(sq, -i, i);

                if (tsq == CORTEX_SQUARE_NULL) break;

                char tpce = pos->board[tsq];

                if (tpce) {
                    if (cortex_piece_get_color(tpce) != pos->color_to_move) {
                        /* Potential capture */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                    }

                    break;
                } else {
                    /* Potential move */
                    output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                }
            }

            /* walk down-left */
            for (int i = 1; i <= 7; ++i) {
                cortex_square tsq = cortex_square_offset(sq, -i, -i);

                if (tsq == CORTEX_SQUARE_NULL) break;

                char tpce = pos->board[tsq];

                if (tpce) {
                    if (cortex_piece_get_color(tpce) != pos->color_to_move) {
                        /* Potential capture */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                    }

                    break;
                } else {
                    /* Potential move */
                    output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                }
            }
            break;
        case 'r':
            /* walk right */
            for (int x = cur_file + 1; x <= 8; ++x) {
                cortex_square tsq = CORTEX_SQUARE_AT(cur_rank, x);
                char tpce = pos->board[tsq];

                if (tpce) {
                    if (cortex_piece_get_color(tpce) != pos->color_to_move) {
                        /* Potential capture */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                    }

                    break;
                } else {
                    /* Potential move */
                    output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                }
            }

            /* walk left */
            for (int x = cur_file - 1; x >= 1; --x) {
                cortex_square tsq = CORTEX_SQUARE_AT(cur_rank, x);
                char tpce = pos->board[tsq];

                if (tpce) {
                    if (cortex_piece_get_color(tpce) != pos->color_to_move) {
                        /* Potential capture */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                    }

                    break;
                } else {
                    /* Potential move */
                    output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                }
            }

            /* walk up */
            for (int y = cur_rank + 1; y <= 8; ++y) {
                cortex_square tsq = CORTEX_SQUARE_AT(y, cur_file);
                char tpce = pos->board[tsq];

                if (tpce) {
                    if (cortex_piece_get_color(tpce) != pos->color_to_move) {
                        /* Potential capture */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                    }

                    break;
                } else {
                    /* Potential move */
                    output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                }
            }

            /* walk down */
            for (int y = cur_rank - 1; y >= 1; --y) {
                cortex_square tsq = CORTEX_SQUARE_AT(y, cur_file);
                char tpce = pos->board[tsq];

                if (tpce) {
                    if (cortex_piece_get_color(tpce) != pos->color_to_move) {
                        /* Potential capture */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                    }

                    break;
                } else {
                    /* Potential move */
                    output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                }
            }
            break;
        case 'b':
            /* walk up-right */
            for (int i = 1; i <= 7; ++i) {
                cortex_square tsq = cortex_square_offset(sq, i, i);

                if (tsq == CORTEX_SQUARE_NULL) break;

                char tpce = pos->board[tsq];

                if (tpce) {
                    if (cortex_piece_get_color(tpce) != pos->color_to_move) {
                        /* Potential capture */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                    }

                    break;
                } else {
                    /* Potential move */
                    output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                }
            }

            /* walk up-left */
            for (int i = 1; i <= 7; ++i) {
                cortex_square tsq = cortex_square_offset(sq, i, -i);

                if (tsq == CORTEX_SQUARE_NULL) break;

                char tpce = pos->board[tsq];

                if (tpce) {
                    if (cortex_piece_get_color(tpce) != pos->color_to_move) {
                        /* Potential capture */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                    }

                    break;
                } else {
                    /* Potential move */
                    output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                }
            }

            /* walk down-right */
            for (int i = 1; i <= 7; ++i) {
                cortex_square tsq = cortex_square_offset(sq, -i, i);

                if (tsq == CORTEX_SQUARE_NULL) break;

                char tpce = pos->board[tsq];

                if (tpce) {
                    if (cortex_piece_get_color(tpce) != pos->color_to_move) {
                        /* Potential capture */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                    }

                    break;
                } else {
                    /* Potential move */
                    output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                }
            }

            /* walk down-left */
            for (int i = 1; i <= 7; ++i) {
                cortex_square tsq = cortex_square_offset(sq, -i, -i);

                if (tsq == CORTEX_SQUARE_NULL) break;

                char tpce = pos->board[tsq];

                if (tpce) {
                    if (cortex_piece_get_color(tpce) != pos->color_to_move) {
                        /* Potential capture */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                    }

                    break;
                } else {
                    /* Potential move */
                    output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                }
            }
            break;
        case 'n':
            for (int a = -1; a <= 1; a += 2) for (int b = -2; b <= 2; b += 4) {
                int tsq = cortex_square_offset(sq, a, b);

                if (tsq != CORTEX_SQUARE_NULL) {
                    char tpce = pos->board[tsq];

                    if (!tpce || cortex_piece_get_color(tpce) != pos->color_to_move) {
                        /* Potential move or capture */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                    }
                }

                tsq = cortex_square_offset(sq, b, a);

                if (tsq != CORTEX_SQUARE_NULL) {
                    char tpce = pos->board[tsq];

                    if (!tpce || cortex_piece_get_color(tpce) != pos->color_to_move) {
                        /* Potential move or capture */
                        output_head = _cortex_transition_list_append_simple_move(output_head, pos, sq, tsq, 0);
                    }
                }
            }
            break;
        }
    }

    return output_head;
}

static cortex_transition_entry* _cortex_transition_list_append_simple_move(cortex_transition_entry* head, cortex_position* pos, cortex_square from, cortex_square to, char promote_type) {
    /* Helper function to add a simple move to a transition list. */
    /* A simple move is where only two squares are altered */
    /* The move is assumed to be a legal move or capture. */

    cortex_transition_entry* new_entry = malloc(sizeof *new_entry);
    memset(new_entry, 0, sizeof *new_entry);

    /* Move string will already be NULL-terminated from memset() */
    cortex_square_write(from, &new_entry->transition.movestr[0]);
    cortex_square_write(to, &new_entry->transition.movestr[2]);
    if (promote_type) new_entry->transition.movestr[4] = promote_type;

    /* Allocate resulting position structure */
    new_entry->transition.result = malloc(sizeof *new_entry->transition.result);
    memcpy(new_entry->transition.result, pos, sizeof *pos);

    /* Invert color to move. */
    new_entry->transition.result->color_to_move = cortex_util_colorflip(pos->color_to_move);

    /* Increment fullmove number if it is now white's move. */
    if (new_entry->transition.result->color_to_move == 'w') {
        ++new_entry->transition.result->fullmove_number;
    }

    /* Increment half-move clock unconditionally.. */
    ++new_entry->transition.result->halfmove_clock;

    /* Reset half-move clock to 0 if the move is a capture or pawn move */
    char from_piece = pos->board[from];
    char to_piece = pos->board[to];
    char from_type = cortex_piece_get_type(from_piece);
    char from_color = cortex_piece_get_color(from_piece);

    if (to_piece || from_type == 'p') {
        new_entry->transition.result->halfmove_clock = 0;
    }

    /* Unset en passant target. */
    new_entry->transition.result->en_passant_target = CORTEX_SQUARE_NULL;

    /* Set en passant target if move was just a pawn jump. */
    int rank_diff = cortex_square_rank_num(to) - cortex_square_rank_num(from);

    /* Tricky quick abs? */
    if (from_type == 'p' && (rank_diff * rank_diff == 4)) {
        new_entry->transition.result->en_passant_target = cortex_square_offset(from, rank_diff / 2, 0);
    }

    /* Disable castling if a king move */
    if (from_type == 'k') {
        if (from_color == 'w') {
            new_entry->transition.result->white_castle_kingside = 0;
            new_entry->transition.result->white_castle_queenside = 0;
        } else {
            new_entry->transition.result->black_castle_kingside = 0;
            new_entry->transition.result->black_castle_queenside = 0;
        }
    }

    /* Disable castling if one of the four rooks is moved */
    if (from_type == 'r' && from == 0) {
        new_entry->transition.result->white_castle_queenside = 0;
    }

    if (from_type == 'r' && from == 7) {
        new_entry->transition.result->white_castle_kingside = 0;
    }

    if (from_type == 'r' && from == 56) {
        new_entry->transition.result->black_castle_queenside = 0;
    }

    if (from_type == 'r' && from == 63) {
        new_entry->transition.result->black_castle_kingside = 0;
    }

    /* Move the piece. */
    new_entry->transition.result->board[to] = new_entry->transition.result->board[from];
    new_entry->transition.result->board[from] = 0;

    /* Apply pending promotions */
    if (promote_type) new_entry->transition.result->board[to] = cortex_util_to_color(promote_type, pos->color_to_move);

    /* If capture was on en passant target, remove the captured pawn. */
    if (from_type == 'p' && to == pos->en_passant_target) {
        new_entry->transition.result->board[pos->en_passant_target] = 0;
    }

    /* Update linked list target */
    if (head) new_entry->next = head;
    return new_entry;
}

cortex_transition_entry* _cortex_transition_list_prune_in_check(cortex_transition_entry* head, char color_in_check) {
    /* We need to drop all resulting positions that have <color_in_check> in check. */
    /* Thanks to the cool recursive nature of linked lists we can do this recursively. */

    if (!head) return NULL;

    if (cortex_position_get_color_in_check(head->transition.result, color_in_check)) {
        cortex_transition_entry* tmp = head->next;
        cortex_transition_entry_free(head);
        return tmp;
    } else {
        head->next = _cortex_transition_list_prune_in_check(head->next, color_in_check);
    }

    return head;
}

void cortex_transition_entry_free(cortex_transition_entry* entry) {
    if (!entry) return;

    free(entry->transition.result);
    free(entry);
}

cortex_transition_entry* cortex_transition_list_free(cortex_transition_entry* head) {
    if (!head) return NULL;
    cortex_transition_list_free(head->next);
    free(head);
    return NULL;
}

static cortex_transition_entry* _cortex_transition_list_append_castles(cortex_transition_entry* head, cortex_position* pos) {
    /* Adds legal castling moves */
    cortex_transition_entry* new_entry;

    /* If the color to move is currently in check, they cannot castle. */
    if (cortex_position_get_color_in_check(pos, pos->color_to_move)) return head;

    /* Check: white moves */
    if (pos->color_to_move == 'w') {
        if (pos->white_castle_kingside && pos->board[7] == 'R') {
            /* Potential kingside castle, check f1 and g1 are empty */
            if (!pos->board[CORTEX_SQUARE_AT(1, 6)] && !pos->board[CORTEX_SQUARE_AT(1, 7)]) {
                /* Check f1 and g1 are not under attack */
                if (!cortex_position_get_square_in_check(pos, CORTEX_SQUARE_AT(1, 6), 'w') && !cortex_position_get_square_in_check(pos, CORTEX_SQUARE_AT(1, 7), 'w')) {
                    /* Looks OK. Generate the move */

                    /* Initialize transition entry */
                    new_entry = malloc(sizeof *new_entry);
                    memset(new_entry, 0, sizeof *new_entry);

                    /* Initialize resulting position */
                    new_entry->transition.result = malloc(sizeof *pos);
                    memcpy(new_entry->transition.result, pos, sizeof *pos);

                    /* Modify resulting state quickly */
                    new_entry->transition.result->board[4] = 0;
                    new_entry->transition.result->board[5] = 'R';
                    new_entry->transition.result->board[6] = 'K';
                    new_entry->transition.result->board[7] = 0;

                    new_entry->transition.result->white_castle_kingside = 0;
                    new_entry->transition.result->white_castle_queenside = 0;

                    /* Write movestr */
                    cortex_square_write(CORTEX_SQUARE_AT(1, 5), &new_entry->transition.movestr[0]);
                    cortex_square_write(CORTEX_SQUARE_AT(1, 7), &new_entry->transition.movestr[2]);

                    /* Insert into list */
                    new_entry->next = head;
                    head = new_entry;
                }
            }
        }
        
        if (pos->white_castle_queenside && pos->board[0] == 'R') {
            /* Potential queenside castle, check b1, c1, d1 are empty */
            if (!pos->board[CORTEX_SQUARE_AT(1, 2)] && !pos->board[CORTEX_SQUARE_AT(1, 3)] && !pos->board[CORTEX_SQUARE_AT(1, 4)]) {
                /* Check c1 and d1 are not under attack */
                if (!cortex_position_get_square_in_check(pos, CORTEX_SQUARE_AT(1, 3), 'w') && !cortex_position_get_square_in_check(pos, CORTEX_SQUARE_AT(1, 4), 'w')) {
                    /* Looks OK. Generate the move */

                    /* Initialize transition entry */
                    new_entry = malloc(sizeof *new_entry);
                    memset(new_entry, 0, sizeof *new_entry);

                    /* Initialize resulting position */
                    new_entry->transition.result = malloc(sizeof *pos);
                    memcpy(new_entry->transition.result, pos, sizeof *pos);

                    /* Modify resulting state quickly */
                    new_entry->transition.result->board[0] = 0;
                    new_entry->transition.result->board[1] = 0;
                    new_entry->transition.result->board[2] = 'K';
                    new_entry->transition.result->board[3] = 'R';
                    new_entry->transition.result->board[4] = 0;

                    new_entry->transition.result->white_castle_kingside = 0;
                    new_entry->transition.result->white_castle_queenside = 0;

                    /* Write movestr */
                    cortex_square_write(CORTEX_SQUARE_AT(1, 5), &new_entry->transition.movestr[0]);
                    cortex_square_write(CORTEX_SQUARE_AT(1, 3), &new_entry->transition.movestr[2]);

                    /* Insert into list */
                    new_entry->next = head;
                    head = new_entry;
                }
            }
        }
    }

    /* Check: black moves */
    if (pos->color_to_move == 'b') {
        if (pos->black_castle_kingside && pos->board[63] == 'r') {
            /* Potential kingside castle, check f8 and g8 are empty */
            if (!pos->board[CORTEX_SQUARE_AT(8, 6)] && !pos->board[CORTEX_SQUARE_AT(8, 7)]) {
                /* Check f8 and g8 are not under attack */
                if (!cortex_position_get_square_in_check(pos, CORTEX_SQUARE_AT(8, 6), 'b') && !cortex_position_get_square_in_check(pos, CORTEX_SQUARE_AT(8, 7), 'b')) {
                    /* Looks OK. Generate the move */

                    /* Initialize transition entry */
                    new_entry = malloc(sizeof *new_entry);
                    memset(new_entry, 0, sizeof *new_entry);

                    /* Initialize resulting position */
                    new_entry->transition.result = malloc(sizeof *pos);
                    memcpy(new_entry->transition.result, pos, sizeof *pos);

                    /* Modify resulting state quickly */
                    new_entry->transition.result->board[60] = 0;
                    new_entry->transition.result->board[61] = 'r';
                    new_entry->transition.result->board[62] = 'k';
                    new_entry->transition.result->board[63] = 0;

                    new_entry->transition.result->black_castle_kingside = 0;
                    new_entry->transition.result->black_castle_queenside = 0;

                    /* Write movestr */
                    cortex_square_write(CORTEX_SQUARE_AT(8, 5), &new_entry->transition.movestr[0]);
                    cortex_square_write(CORTEX_SQUARE_AT(8, 7), &new_entry->transition.movestr[2]);

                    /* Insert into list */
                    new_entry->next = head;
                    head = new_entry;
                }
            }
        }
        
        if (pos->black_castle_queenside && pos->board[56] == 'r') {
            /* Potential queenside castle, check b8, c8, d8 are empty */
            if (!pos->board[CORTEX_SQUARE_AT(8, 2)] && !pos->board[CORTEX_SQUARE_AT(8, 3)] && !pos->board[CORTEX_SQUARE_AT(8, 4)]) {
                /* Check c8 and d8 are not under attack */
                if (!cortex_position_get_square_in_check(pos, CORTEX_SQUARE_AT(8, 3), 'b') && !cortex_position_get_square_in_check(pos, CORTEX_SQUARE_AT(8, 4), 'b')) {
                    /* Looks OK. Generate the move */

                    /* Initialize transition entry */
                    new_entry = malloc(sizeof *new_entry);
                    memset(new_entry, 0, sizeof *new_entry);

                    /* Initialize resulting position */
                    new_entry->transition.result = malloc(sizeof *pos);
                    memcpy(new_entry->transition.result, pos, sizeof *pos);

                    /* Modify resulting state quickly */
                    new_entry->transition.result->board[56] = 0;
                    new_entry->transition.result->board[57] = 0;
                    new_entry->transition.result->board[58] = 'k';
                    new_entry->transition.result->board[59] = 'r';
                    new_entry->transition.result->board[60] = 0;

                    new_entry->transition.result->black_castle_kingside = 0;
                    new_entry->transition.result->black_castle_queenside = 0;

                    /* Write movestr */
                    cortex_square_write(CORTEX_SQUARE_AT(8, 5), &new_entry->transition.movestr[0]);
                    cortex_square_write(CORTEX_SQUARE_AT(8, 3), &new_entry->transition.movestr[2]);

                    /* Insert into list */
                    new_entry->next = head;
                    head = new_entry;
                }
            }
        }
    }

    return head;
}
