#include "../types.h"

#include <string.h>

rc cmd_func_cwd(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc != 1) return ERR_ARGC;
    if (!fs) return ERR_INTERN;
    if (!fs->file) return ERR_INTERN;

    // root dir
    if (!*cluster) {
        printf("/");
        return SUCCESS;
    }

    // variables
    bool success    = false;
    uint32_t i      = 0;
    file_item vItem = {0};

    uint32_t par_cluster = 0;

    // find parent cluster
    fseek(fs->file, fs->header.data_start_address + (*cluster) * fs->header.cluster_size, SEEK_SET);
    for (i = 0; i < fs->header.cluster_size; i += sizeof(file_item)) {
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;

        if (!strcmp(vItem.name, ".."))
            par_cluster = vItem.start_cluster;
    }

    // find current directory in parent
    fseek(fs->file, fs->header.data_start_address + par_cluster * fs->header.cluster_size, SEEK_SET);
    for (i = 0; i < fs->header.cluster_size; i += sizeof(file_item)) {
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;
        if (vItem.start_cluster == *cluster) break;
    }

    // print cwd
    printf("%s", vItem.name);

    return SUCCESS;
}
