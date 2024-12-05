#include "../types.h"
#include "utils.h"

#include <string.h>

static rc _cd(file_system *fs, uint32_t *cluster, char *path);

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

    char path_arr[MAX_PATH_NUM][BUFFER_SIZE] = {0};
    int i = parse_path(path, path_arr);
    int j = 0;
    uint32_t cls = *cluster;

    if (path_arr[0][0] == '/') cls = 0;

    for (j = 0; j < i; j++) {
        rc rc = _cd(fs, &cls, path_arr[j]);
        if (rc != SUCCESS) return rc;
    }

    *cluster = cls;
    return SUCCESS;
}

rc _cd(file_system *fs, uint32_t *cluster, char *path) {
    // variables
    bool success    = false;
    uint32_t i      = 0;
    file_item vItem = {0};

    // find directory to cd into in the current cluster
    fseek(fs->file, fs->header.data_start_address + (*cluster) * fs->header.cluster_size, SEEK_SET);
    for (i = 0; i < fs->header.cluster_size; i += sizeof(file_item)) {
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ; // could an entry be read
        if (!vItem.name[0]) continue;            // does entry have a name
        if (strcmp(path, vItem.name)) continue;  // does entry name match
        if (vItem.size != FAT_DIRECTORY) break;  // is entry a directory
        *cluster = vItem.start_cluster;
        return SUCCESS;
    }

    return ERR_ARGV;
}
