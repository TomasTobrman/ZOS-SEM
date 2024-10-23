#include "../fat.h"
#include "../types.h"

rc cmd_func_mkdir(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc != 2) return ERR_ARGC;
    if (!fs) return ERR_INTERN;

    // TODO: check name validity
    // TODO: check for existing file name

    return fat_mkdir(fs, *cluster, argv[1]);
}
