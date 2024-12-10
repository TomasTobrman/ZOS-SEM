#include "utils.h"
#include "types.h"

#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

/* ---- commands ---- */
static command commands[] = {
    /* ---- default commands ---- */
    {.cmd="cat"   , .func=cmd_func_cat   },
    {.cmd="cd"    , .func=cmd_func_cd    },
    {.cmd="cp"    , .func=cmd_func_cp    },
    {.cmd="format", .func=cmd_func_format},
    {.cmd="incp"  , .func=cmd_func_incp  },
    {.cmd="info"  , .func=cmd_func_info  },
    {.cmd="load"  , .func=cmd_func_load  },
    {.cmd="ls"    , .func=cmd_func_ls    },
    {.cmd="mkdir" , .func=cmd_func_mkdir },
    {.cmd="mv"    , .func=cmd_func_mv    },
    {.cmd="outcp" , .func=cmd_func_outcp },
    {.cmd="pwd"   , .func=cmd_func_pwd   },
    {.cmd="rm"    , .func=cmd_func_rm    },
    {.cmd="rmdir" , .func=cmd_func_rmdir },
    /* ---- additional commands ---- */
    // {.cmd="defrag", .func=cmd_func_defrag},
    /* ---- custom commands ---- */
    {.cmd="clear" , .func=cmd_func_clear },
    {.cmd="exit"  , .func=cmd_func_exit  },
    {.cmd="help"  , .func=cmd_func_help  },
};
static int8_t n_commands = sizeof(commands) / sizeof(command);

void report_rc(const char *reporter, rc rc) {
    if (rc & ERR_ARGC)
        fprintf(stderr, "%s: wrong number of arguments.\n  try running the \"help\" command.\n", reporter);
    if (rc & ERR_ARGV)
        fprintf(stderr, "%s: invalid arguments.\n  try running the \"help\" command.\n", reporter);

    if (rc & ERR_READ)
        fprintf(stderr, "%s: could not read from file.\n", reporter);
    if (rc & ERR_WRITE)
        fprintf(stderr, "%s: could not write into file.\n", reporter);

    if (rc & ERR_NOEMPTY)
        fprintf(stderr, "%s: directory is not empty.\n", reporter);

    if (rc & ERR_CMD && rc & ERR_NOEXISTS)
        fprintf(stderr, "%s: No such file or directory\n", reporter);

    if (rc & ERR_FAT && rc & ERR_CREATE)
        fprintf(stderr, "%s: could not create DAT file.\n", reporter);
    if (rc & ERR_FAT && rc & ERR_OPEN)
        fprintf(stderr, "%s: could not open DAT file.\n  try running the \"format\" command to create it.\n", reporter);
    if (rc & ERR_FAT && rc & ERR_FULL)
        fprintf(stderr, "%s: file-system is full.\n", reporter);

    if (rc & ERR_INTERN && rc & ERR_MEMORY)
        fprintf(stderr, "%s: internal memory error.\n", reporter);
}

char *trimwhitespace(char *str) {
    char *end;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

void prepend(char *s, const char *t) {
    size_t len = strlen(t);
    memmove(s + len, s, strlen(s) + 1);
    memcpy(s, t, len);
}

int parse_path(char *path, char result[MAX_PATH_NUM][BUFFER_SIZE]) {
    int i = 0, j = 0;
    char *str = 0;

    str = strtok(path, "/");
    while (str != 0 && i < 32) {
        if (!str[0]) continue; // empty
        int len = strlen(str);
        for (j = 0; j < len && j < BUFFER_SIZE; j++)
            result[i][j] = str[j];
        i++;
        str = strtok(0, "/");
    }

    return i;
}

rc parse_command(file_system *fs, uint32_t *cluster, char *buffer) {
    // variables
    bool cmd_valid           = false;
    char *buffer_trimmed     = {0};
    char *token              = {0};
    int i                    = 0;
    int j                    = 0;
    char *args[ARGS_SIZE]    = {0};
    rc   return_code         = 0;

    // trim whitespace from the input buffer
    buffer_trimmed = trimwhitespace(buffer);

    // is empty line
    if (*buffer_trimmed == 0) return SUCCESS;

    // separate command from potential arguments
    token = strtok(buffer_trimmed, " ");
    if (!token) token = buffer_trimmed;

    // find command in array and execute it
    cmd_valid = false;
    for (i = 0; i < n_commands; i++) {
        if (strcmp(token, commands[i].cmd)) continue;

        // split arguments and add them to an array
        memset(args, 0, ARGS_SIZE * sizeof(char*));
        for (j = 0; token && j < ARGS_SIZE; j++) {
            args[j] = token;
            token = strtok(NULL, " ");
        }

        // execute command
        return_code = commands[i].func(fs, cluster, j, args);
        cmd_valid = true;
        break;
    }

    // input does not match any of the valid commands
    if (!cmd_valid)
        fprintf(stderr, "\"%s\" is not a valid command.\n", token);

    return return_code;
}
