#include "../fat.h"
#include "../types.h"
#include "../utils.h"

#include <stdio.h>
#include <string.h>

rc cmd_func_pwd(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc != 1) return ERR_ARGC;
    if (!fs) return ERR_INTERN;
    if (!fs->file) return ERR_INTERN;

    // root dir
    if (!*cluster) {
        printf("/\n");
        return SUCCESS;
    }

    // get pwd
    char buffer_pwd[BUFFER_SIZE] = {0};
    char buffer_cwd[BUFFER_SIZE] = {0};
    uint32_t cls = *cluster;
    while (cls) {
        memset(buffer_cwd, 0, BUFFER_SIZE);
        fat_cwd(fs, &cls, buffer_cwd);
        prepend(buffer_cwd, "/");
        prepend(buffer_pwd, buffer_cwd);
    }

    // print pwd
    printf("%s\n", buffer_pwd);

    return SUCCESS;
}
