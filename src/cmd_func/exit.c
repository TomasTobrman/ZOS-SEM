#include "../types.h"

#include <stdlib.h>

rc cmd_func_exit(fat *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc > 2) return ERR_ARGC;
    fprintf(stdout, "exit\n");
    return CMD_EXIT | (argc == 2 ? (uint8_t)atoi(argv[1]) : 0);
}
