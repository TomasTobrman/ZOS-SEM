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
    bool running             = true;
    int i                    = 0;
    int exit_code            = 0;
    rc  return_code          = 0;
    file_system *fs          = 0;
    uint32_t current_cluster = 0;
    char buffer[BUFFER_SIZE] = {0};
    char in_char             = 0;

    const char *username     = getlogin();
    if (!username) username  = "user";
    const char *hostname     = "tfs";

    /* ---- file-system ---- */
    return_code = fat_open(&fs, argv[1]);
    report_rc(hostname, return_code);

    /* ---- loop ---- */
    while (running) {
        // prompt
        printf("[%s@%s ", username, hostname);
        cmd_func_cwd(fs, &current_cluster, 1, NULL);
        printf("]$ ");

        // get user input
        memset(buffer, 0, BUFFER_SIZE);
        for (i = 0; i < BUFFER_SIZE; i++) {
            in_char = getchar();
            if (in_char == '\n' || in_char == EOF) break;
            buffer[i] = in_char;
        }

        return_code = parse_command(fs, &current_cluster, buffer);
        if (return_code & CMD_EXIT) {
            exit_code = return_code & 0xFF;
            running = false;
        } else report_rc(buffer, return_code);
    }

    /* ---- close ---- */
    fat_close(&fs);

    return exit_code;
}
