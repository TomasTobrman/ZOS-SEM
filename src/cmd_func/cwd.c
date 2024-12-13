#include "fat.h"
#include "types.h"

rc cmd_func_cwd(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc != 1) return ERR_ARGC;
    if (!fs) return ERR_INTERN;
    if (!fs->file) return ERR_INTERN;

    // root dir
    if (!*cluster) {
        printf("/");
        return SUCCESS;
    }

    // get cwd
    char buffer[BUFFER_SIZE] = {0};
    uint32_t cls = *cluster;
    fat_cwd(fs, &cls, buffer);

    // print cwd
    printf("%s", buffer);

    return SUCCESS;
}
