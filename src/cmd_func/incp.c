#include "types.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

rc cmd_func_incp(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc != 3) return ERR_ARGC;
    if (!fs) return ERR_INTERN;
    if (!fs->file) return ERR_FAT | ERR_NOEXISTS;

    // variables
    bool success        = false;
    FILE *file          = 0;
    size_t file_size    = 0;
    size_t n_read       = 0;
    uint32_t n_clusters = 0;
    uint32_t cls        = 0;
    uint32_t cls_next   = 0;
    uint32_t i          = 0;
    uint32_t j          = 0;
    uint32_t clusters[fs->header.cluster_count];
    char buffer[fs->header.cluster_size];
    file_item vItem     = {0};

    // check if file exists
    file = fopen(argv[1], "r");
    if (!file) return ERR_FILE | ERR_OPEN;

    // get size of file
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    n_read = file_size;
    fseek(file, 0, SEEK_SET);

    // calculate number of clusters needed
    n_clusters = file_size / fs->header.cluster_size + (file_size % fs->header.cluster_size ? 1 : 0);

    // check if file fits
    fseek(fs->file, fs->header.fat_start_address, SEEK_SET);
    for (i = 0; i < fs->header.cluster_count; i++) {
        success = fread(&cls, sizeof(uint32_t), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;
        if (cls == FAT_UNUSED) clusters[j++] = i;
    } if (j < n_clusters) return ERR_FAT | ERR_FULL;

    for (i = 0; i < n_clusters; i++) {
        // set value in table
        fseek(fs->file, fs->header.fat_start_address + clusters[i] * sizeof(uint32_t), SEEK_SET);
        cls_next = i + 1 < n_clusters ? clusters[i + 1] : FAT_FILE_END;
        success = fwrite(&cls_next, sizeof(uint32_t), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_WRITE;

        // read in the file by clusters
        memset(buffer, 0, fs->header.cluster_size);
        success = fread(buffer, n_read >= fs->header.cluster_size ? fs->header.cluster_size : n_read, 1, file) == 1;
        n_read -= fs->header.cluster_size;
        if (!success) return ERR_FILE | ERR_READ;

        // write file clusters to data
        fseek(fs->file, fs->header.data_start_address + clusters[i] * fs->header.cluster_size, SEEK_SET);
        success = fwrite(buffer, fs->header.cluster_size, 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_WRITE;
    }

    fseek(fs->file, fs->header.data_start_address + (*cluster) * fs->header.cluster_size, SEEK_SET);
    for (i = 0; i < fs->header.cluster_size; i += sizeof(file_item)) {
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;
        if (vItem.name[0]) continue;

        strcpy(vItem.name, argv[2]);
        vItem.size = file_size;
        vItem.start_cluster = clusters[0];

        fseek(fs->file, fs->header.data_start_address + (*cluster) * fs->header.cluster_size + i - sizeof(file_item), SEEK_SET);
        success = fwrite(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_WRITE;
        break;
    } if (i >= fs->header.cluster_size) return ERR_ARGV;

    printf("done\n");
    return SUCCESS;
}
