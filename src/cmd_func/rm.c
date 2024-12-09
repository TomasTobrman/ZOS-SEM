#include "types.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

rc cmd_func_rm(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc != 2) return ERR_ARGC;
    if (!fs) return ERR_INTERN;
    if (!fs->file) return ERR_FAT | ERR_NOEXISTS;

    // cannot remove "." or ".." or "/" directories
    if (!strcmp(argv[1], ".")) return ERR_ARGV;
    if (!strcmp(argv[1], "..")) return ERR_ARGV;
    if (!strcmp(argv[1], "/")) return ERR_ARGV;

    // variables
    bool success      = false;
    uint32_t cls      = 0;
    uint32_t cls_next = 0;
    uint32_t i        = 0;
    file_item vItem   = {0};
    char buffer[fs->header.cluster_size];
    memset(buffer, 0, fs->header.cluster_size);

    // find directory by name
    cls = *cluster;
    fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size, SEEK_SET);
    for (i = 0; i < fs->header.cluster_size; i += sizeof(file_item)) {
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;

        if (!vItem.name[0]) continue;
        if (strcmp(vItem.name, argv[1])) continue;
        if (vItem.size == FAT_DIRECTORY) return ERR_ARGV;
        break;
    } if (i >= fs->header.cluster_size) return ERR_ARGV;

    // remove data file entry
    fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size + i, SEEK_SET);
    cls = vItem.start_cluster;
    memset(&vItem, 0, sizeof(file_item));
    success = fwrite(&vItem, sizeof(file_item), 1, fs->file) == 1;
    if (!success) return ERR_FAT | ERR_WRITE;

    // traverse clusters
    while (cls != FAT_FILE_END) {
        // get next table entry index
        fseek(fs->file, fs->header.fat_start_address + cls * sizeof(uint32_t), SEEK_SET);
        success = fread(&cls_next, sizeof(uint32_t), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;

        // clear table entry
        fseek(fs->file, fs->header.fat_start_address + cls * sizeof(uint32_t), SEEK_SET);
        i = FAT_UNUSED;
        success = fwrite(&i, sizeof(uint32_t), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_WRITE;

        // clear data cluster
        fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size, SEEK_SET);
        success = fwrite(buffer, fs->header.cluster_size, 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_WRITE;

        // go to next cluster
        cls = cls_next;
    }

    return SUCCESS;
}
