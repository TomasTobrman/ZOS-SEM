#include "types.h"

#include <stdio.h>
#include <string.h>

rc cmd_func_cat(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc != 2) return ERR_ARGC;
    if (!fs) return ERR_INTERN;
    if (!fs->file) return ERR_FAT | ERR_NOEXISTS;

    // variables
    bool success    = false;
    size_t n_read   = 0;
    uint32_t i      = 0;
    uint32_t cls    = 0;
    file_item vItem = {0};
    char buffer[fs->header.cluster_size + 1];

    // find entry
    fseek(fs->file, fs->header.data_start_address + (*cluster) * fs->header.cluster_size, SEEK_SET);
    for (i = 0; i < fs->header.cluster_size; i += sizeof(file_item)) {
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;

        if (!vItem.name[0]) continue;
        if (strcmp(vItem.name, argv[1])) continue;
        if (vItem.size == FAT_DIRECTORY) return ERR_ARGV;

        n_read = vItem.size;
        cls = vItem.start_cluster;
        break;
    } if (i >= fs->header.cluster_size) return ERR_ARGV;

    // traverse clusters
    while (cls != FAT_FILE_END) {
        // read clusters and print contents
        memset(buffer, 0, fs->header.cluster_size + 1);
        fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size, SEEK_SET);
        success = fread(buffer, n_read >= fs->header.cluster_size ? fs->header.cluster_size : n_read, 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;
        n_read -= fs->header.cluster_size;
        printf("%s", buffer);

        // get next cluster
        fseek(fs->file, fs->header.fat_start_address + cls * sizeof(uint32_t), SEEK_SET);
        success = fread(&cls, sizeof(uint32_t), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;
    } printf("\n");

    return SUCCESS;
}
