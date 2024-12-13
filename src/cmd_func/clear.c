#include "types.h"

#include <stdlib.h>

rc cmd_func_clear(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc != 1) return ERR_ARGC;
    system("clear");
    return SUCCESS;
}
