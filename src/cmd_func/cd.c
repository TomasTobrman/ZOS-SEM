#include "../types.h"

#include <string.h>

rc cmd_func_cd(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc > 2) return ERR_ARGC;
    if (!fs) return ERR_INTERN;
    if (!fs->file) return ERR_INTERN;

    char *path = argc == 1 ? "/" : argv[1];

    // change directory to root
    if (!strcmp(path, "/")) {
        *cluster = 0;
        return SUCCESS;
    }

    // variables
    bool success    = false;
    uint32_t i      = 0;
    file_item vItem = {0};

    // find directory to cd into in the current cluster
    fseek(fs->file, fs->header.data_start_address + (*cluster) * fs->header.cluster_size, SEEK_SET);
    for (i = 0; i < fs->header.cluster_size; i += sizeof(file_item)) {
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;
        if (!vItem.name[0]) continue;
        if (!strcmp(path, vItem.name)) {
            *cluster = vItem.start_cluster;
            return SUCCESS;
        }
    }

    return ERR_ARGV;
}
