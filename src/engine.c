#include "engine.h"
#include "log.h"
#include "eval.h"

#include <string.h>

int cortex_engine_init(cortex_engine* eng, cortex_options* opts) {
    if (!eng) return -1;

    memset(eng, 0, sizeof *eng);
    eng->opts = opts;

    return cortex_position_standard(&eng->position, NULL);
}

void cortex_engine_free(cortex_engine* eng) {
}

int cortex_engine_run(cortex_engine* eng) {
    if (!eng) return -1;

    /* disable buffering */
    setvbuf(eng->opts->out, NULL, _IONBF, 0);

    /* expect a 'uci' handshake */
    cortex_engine_read_line(eng);

    if (strcmp(eng->input_buf, "uci\n")) {
        /* Chop the newline */
        eng->input_buf[strlen(eng->input_buf)-1] = 0;
        cortex_error("Expected 'uci', read '%s'", eng->input_buf);
        return -1;
    }

    /* send back identification */
    fprintf(eng->opts->out, "id name " CORTEX_ID_NAME "\n");
    fprintf(eng->opts->out, "id author " CORTEX_ID_AUTHOR "\n");

    /* send back a 'uciok' */
    fprintf(eng->opts->out, "uciok\n");

    cortex_log("Sent back uciok.");

    /* wait for commands until input broken */
    while (!cortex_engine_read_line(eng)) {
        cortex_log("Received command: %s", eng->input_buf);

        char* inp_save;
        char* cmd = strtok_r(eng->input_buf, " \n", &inp_save);

        if (!cmd) continue;

        if (!strcmp(cmd, "isready")) {
            fprintf(eng->opts->out, "readyok\n");
        } else if (!strcmp(cmd, "debug")) {
            char* arg = strtok_r(NULL, " \n", &inp_save);

            if (!strcmp(arg, "on")) {
                eng->debug = 1;
                cortex_log("Enabled engine debugging");
            } else if (!strcmp(arg, "off")) {
                eng->debug = 0;
                cortex_log("Disabled engine debugging");
            } else {
                cortex_error("Unrecognized argument '%s' to 'debug'!", arg);
            }
        } else if (!strcmp(cmd, "position")) {
            char* line = strtok_r(NULL, "\n", &inp_save);
            char* moves = NULL;

            if (!line) line = "startpos";

            if (!strncmp(line, "startpos", 7)) {
                moves = line + 8;
                if (*moves == ' ') ++moves;

                cortex_position_standard(&eng->position, moves);
                cortex_log("Loaded standard position, moves %s", moves);
            } else {
                /* Line has been split. Load the position */
                int ret = 0;
                if ((ret = cortex_position_from_fen(&eng->position, line))) {
                    cortex_error("Failed to load position from FEN %s : %d", line, ret);
                    return ret;
                }

                cortex_log("Loaded position from FEN+moves %s", line);

                char fen[CORTEX_POSITION_FEN_BUFLEN] = {0};
                cortex_position_write_fen(&eng->position, fen);
                cortex_log("Position export: %s", fen);
            }
        } else if (!strcmp(cmd, "go")) {
            cortex_eval_go(&eng->position, eng->opts->out);
        } else if (!strcmp(cmd, "quit")) {
            cortex_log("Received quit request.");
            return 0;
        } else if (!strcmp(cmd, "stop")) {
            cortex_log("Received stop request.");
        } else {
            cortex_error("error: unknown UCI command '%s'\n", cmd);
        }
    }

    return 0;
}

int cortex_engine_read_line(cortex_engine* eng) {
    if (!eng) return -1;

    if (!fgets(eng->input_buf, sizeof eng->input_buf, eng->opts->in)) {
        return -1;
    }

    return 0;
}
