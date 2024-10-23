#include "types.h"
#include "utils.h"
#include "fat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void usage(const char *msg) {
    fprintf(stderr, "tfs: %s\nusage: tfs <fs>\n  <fs>    File-System file path.\n", msg);
}

static void prompt(const char *username, const char *hostname, const char *cwd) {
    printf("[%s@%s %s]$ ", username, hostname, cwd);
}

int main(int argc, char **argv) {
    /* ---- check arguments ---- */
    if (argc < 2) {
        usage("missing arguments");
        return 1;
    }

    if (argc > 2) {
        usage("too many arguments");
        return 1;
    }

    /* ---- variables ---- */
    int i                    = 0;
    int j                    = 0;

    int  ec                  = 0;
    rc   rc                  = 0;

    bool running             = true;
    bool cmd_valid           = false;

    fat *fs                  = 0;
    uint32_t cc              = 0;

    char buffer[BUFFER_SIZE] = {0};
    char *buffer_trimmed     = {0};
    char *token              = {0};
    char in_c                = 0;

    char *args[ARGS_SIZE]    = {0};

    const char *username     = getlogin();
    const char *hostname     = "tfs";
    char       *cwd          = "/";

    /* ---- commands ---- */
    command cmds[] = {
        /* ---- default commands ---- */
        // {.cmd="cat"   , .func=cmd_func_cat   },
        // {.cmd="cd"    , .func=cmd_func_cd    },
        // {.cmd="cp"    , .func=cmd_func_cp    },
        {.cmd="format", .func=cmd_func_format},
        // {.cmd="incp"  , .func=cmd_func_incp  },
        // {.cmd="info"  , .func=cmd_func_info  },
        // {.cmd="load"  , .func=cmd_func_load  },
        // {.cmd="ls"    , .func=cmd_func_ls    },
        // {.cmd="mkdir" , .func=cmd_func_mkdir },
        // {.cmd="mv"    , .func=cmd_func_mv    },
        // {.cmd="outcp" , .func=cmd_func_outcp },
        // {.cmd="pwd"   , .func=cmd_func_pwd   },
        // {.cmd="rm"    , .func=cmd_func_rm    },
        // {.cmd="rmdir" , .func=cmd_func_rmdir },
        /* ---- additional commands ---- */
        // {.cmd="defrag", .func=cmd_func_defrag},
        /* ---- custom commands ---- */
        {.cmd="clear" , .func=cmd_func_clear },
        {.cmd="exit"  , .func=cmd_func_exit  },
        {.cmd="help"  , .func=cmd_func_help  },
    };
    int8_t n_cmd = sizeof(cmds) / sizeof(command);

    /* ---- file-system ---- */
    rc = fat_open(&fs, argv[1]);
    report_rc(hostname, rc);

    /* ---- loop ---- */
    while (running) {
        // get user input
        memset(buffer, 0, BUFFER_SIZE);
        prompt(username, hostname, cwd);
        for (i = 0; i < BUFFER_SIZE; i++) {
            in_c = getchar();
            if (in_c == '\n' || in_c == EOF) break;
            buffer[i] = in_c;
        }

        // trim whitespace from the input buffer
        buffer_trimmed = trimwhitespace(buffer);

        // is empty line
        if (*buffer_trimmed == 0) continue;

        // separate command from potential arguments
        token = strtok(buffer_trimmed, " ");
        if (!token) token = buffer_trimmed;

        // find command in array and execute it
        cmd_valid = false;
        for (i = 0; i < n_cmd; i++) {
            if (strcmp(token, cmds[i].cmd)) continue;

            // split arguments and add them to an array
            memset(args, 0, ARGS_SIZE * sizeof(char*));
            for (j = 0; token && j < ARGS_SIZE; j++) {
                args[j] = token;
                token = strtok(NULL, " ");
            }

            // execute command
            rc = cmds[i].func(fs, &cc, j, args);
            if (rc & CMD_EXIT) {
                ec = rc & 0xFF;
                running = false;
            } else report_rc(args[0], rc);

            cmd_valid = true;
            break;
        }

        // input does not match any of the valid commands
        if (!cmd_valid)
            fprintf(stderr, "\"%s\" is not a valid command.\n", token);
    }

    /* ---- close ---- */
    fat_close(&fs);

    return ec;
}
