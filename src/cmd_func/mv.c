#include "types.h"

#include <string.h>

rc cmd_func_mv(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc != 3) return ERR_ARGC;
    if (!fs) return ERR_INTERN;
    if (!fs->file) return ERR_FAT | ERR_NOEXISTS;

    // cannot remove "." or ".." or "/" directories
    if (!strcmp(argv[1], ".")) return ERR_ARGV;
    if (!strcmp(argv[1], "..")) return ERR_ARGV;
    if (!strcmp(argv[1], "/")) return ERR_ARGV;

    // variables
    bool success          = false;
    bool rename           = false;
    uint32_t cls          = 0;
    uint32_t cls_out      = 0;
    uint32_t i            = 0;
    uint32_t j            = 0;
    file_item vItem       = {0};

    // find out entry by name
    cls = *cluster;
    fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size, SEEK_SET);
    for (i = 0; i < fs->header.cluster_size; i += sizeof(file_item)) {
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;
        if (!vItem.name[0]) continue;
        if (strcmp(vItem.name, argv[2])) continue;
        if (vItem.size != FAT_DIRECTORY) return ERR_ARGV;
        cls_out = vItem.start_cluster;
        break;
    } if (i >= fs->header.cluster_size) rename = true;

    // find in entry by name
    fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size, SEEK_SET);
    for (i = 0; i < fs->header.cluster_size; i += sizeof(file_item)) {
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;
        if (!vItem.name[0]) continue;
        if (strcmp(vItem.name, argv[1])) continue;
        break;
    } if (i >= fs->header.cluster_size) return ERR_ARGV;

    // just rename entry
    if (rename) {
        fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size + i, SEEK_SET);
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;

        strcpy(vItem.name, argv[2]);

        fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size + i, SEEK_SET);
        success = fwrite(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_WRITE;

        return SUCCESS;
    }

    // move entry to different directory
    for (j = 0; j < fs->header.cluster_size; j += sizeof(file_item)) {
        // find empty space in out directory
        fseek(fs->file, fs->header.data_start_address + cls_out * fs->header.cluster_size + j, SEEK_SET);
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;
        if (vItem.name[0]) continue;

        // copy the in entry
        fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size + i, SEEK_SET);
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;

        // write copy of the in entry
        fseek(fs->file, fs->header.data_start_address + cls_out * fs->header.cluster_size + j, SEEK_SET);
        success = fwrite(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_WRITE;

        // remove the last in entry
        memset(&vItem, 0, sizeof(file_item));
        fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size + i, SEEK_SET);
        success = fwrite(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_WRITE;

        break;
    } if (i >= fs->header.cluster_size) return ERR_FAT | ERR_FULL;

    return SUCCESS;
}
