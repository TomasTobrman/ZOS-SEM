#include "../types.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

rc cmd_func_rmdir(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc != 2) return ERR_ARGC;
    if (!fs) return ERR_INTERN;

    // cannot remove "." or ".." or "/" directories
    if (!strcmp(argv[1], ".")) return ERR_ARGV;
    if (!strcmp(argv[1], "..")) return ERR_ARGV;
    if (!strcmp(argv[1], "/")) return ERR_ARGV;

    bool success    = false;
    bool found      = false;
    uint32_t i      = 0;
    uint32_t j      = 0;
    file_item vItem = {0};

    uint32_t cls = 0;

    // TODO: parse path

    // find directory by name
    fseek(fs->file, fs->header.data_start_address + (*cluster) * fs->header.cluster_size, SEEK_SET);
    for (i = 0; i < fs->header.cluster_size; i += sizeof(file_item)) {
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;

        if (!vItem.name[0]) continue;
        if (strcmp(vItem.name, argv[1])) continue;
        if (vItem.size != FAT_DIRECTORY) return ERR_ARGV;

        cls = vItem.start_cluster;

        found = true;
        break;
    }
    if (!found) return ERR_ARGV;

    // check if dir empty
    fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size + 2 * sizeof(file_item), SEEK_SET);
    for (j = 2; j < fs->header.cluster_size; j += sizeof(file_item)) {
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;
        if (vItem.name[0]) return ERR_NOEMPTY;
    }

    // remove data entry
    file_item item_empty = {0};
    fseek(fs->file, fs->header.data_start_address + (*cluster) * fs->header.cluster_size + i, SEEK_SET);
    success = fwrite(&item_empty, sizeof(file_item), 1, fs->file) == 1;
    if (!success) return ERR_FAT | ERR_WRITE;

    // remove table entry
    fseek(fs->file, fs->header.fat_start_address + cls * sizeof(uint32_t), SEEK_SET);
    uint32_t val_unused = FAT_UNUSED;
    success = fwrite(&val_unused, sizeof(uint32_t), 1, fs->file) == 1;
    if (!success) return ERR_FAT | ERR_WRITE;

    return SUCCESS;
}
