#include "types.h"
#include "fat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

rc fat_open(fat **fs, const char *fp) {
    if (!fs) return ERR_INTERN;

    *fs = (fat*)malloc(sizeof(fat));
    if (!*fs) return ERR_INTERN | ERR_MEMORY;

    memset(*fs, 0, sizeof(fat));
    (*fs)->filepath = fp;
    (*fs)->file = fopen((*fs)->filepath, "r+b");
    if (!(*fs)->file) return ERR_FAT | ERR_OPEN;

    return SUCCESS;
}

rc fat_close(fat **fs) {
    if (!fs || !*fs) return ERR_INTERN;

    if ((*fs)->file) {
        fclose((*fs)->file);
        (*fs)->file = NULL;
    }

    free(*fs);
    *fs = NULL;

    return SUCCESS;
}

rc fat_format(fat *fs, uint32_t size) {
    if (!fs) return ERR_INTERN;

    fs->desc.signature[0]       = 't';
    fs->desc.signature[1]       = 't';
    fs->desc.signature[2]       = 'o';
    fs->desc.signature[3]       = 'b';
    fs->desc.signature[4]       = 'r';
    fs->desc.signature[5]       = 'm';
    fs->desc.signature[6]       = 'a';
    fs->desc.signature[7]       = 'n';
    fs->desc.signature[8]       = '\0';

    fs->desc.disk_size          = size;
    fs->desc.cluster_size       = KILOBYTE;

    fs->header_size             = sizeof(fat_desc);

    fs->desc.cluster_count      = (fs->desc.disk_size - fs->header_size) / fs->desc.cluster_size;

    fs->table_size              = sizeof(uint32_t) * fs->desc.cluster_count;

    fs->desc.fat_start_address  = fs->header_size;
    fs->desc.data_start_address = fs->desc.fat_start_address + fs->table_size;

    fs->data_size               = fs->desc.disk_size - fs->desc.data_start_address;

    bool success = false;

    // write zeros into file
    printf("---- FORMATTING ----------------------\n");
    printf(" [%u B]\n"                  , fs->desc.disk_size);

    if (fs->file) fclose(fs->file);
    fs->file = fopen(fs->filepath, "w+b");
    if (!fs->file) return ERR_FAT | ERR_OPEN;

    rewind(fs->file);
    uint8_t byte_val = 0;
    for (int i = 0; i < fs->desc.disk_size; i++) {
        success = fwrite(&byte_val, 1, 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_WRITE;
    }

    // write header
    printf("---- HEADER --------------------------\n");
    printf(" [%u B]\n"                  , fs->header_size);
    printf("  signature          : %s\n", fs->desc.signature);
    printf("  disk size          : %d\n", fs->desc.disk_size);
    printf("  cluster size       : %d\n", fs->desc.cluster_size);
    printf("  cluster count      : %d\n", fs->desc.cluster_count);
    printf("  fat start address  : %d\n", fs->desc.fat_start_address);
    printf("  data start address : %d\n", fs->desc.data_start_address);

    rewind(fs->file);
    success = fwrite(&fs->desc, fs->header_size, 1, fs->file) == 1;
    if (!success) return ERR_FAT | ERR_WRITE;

    // write fat table
    printf("---- TABLE ---------------------------\n");
    printf(" [%u B]\n"                  , fs->table_size);

    uint32_t int_val = FAT_FILE_END;
    fseek(fs->file, 0, fs->desc.fat_start_address);
    success = fwrite(&int_val, sizeof(uint32_t), 1, fs->file) == 1;
    if (!success) return ERR_FAT | ERR_WRITE;
    int_val = FAT_UNUSED;
    for (int i = 1; i < fs->table_size / sizeof(uint32_t); i++) {
        fseek(fs->file, i, fs->desc.fat_start_address);
        success = fwrite(&int_val, sizeof(uint32_t), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_WRITE;
    }

    // write root dir into data
    printf("---- DATA ----------------------------\n");
    printf(" [%u B]\n"                  , fs->data_size);

    fseek(fs->file, 0, fs->desc.data_start_address);
    fat_item root = {.item_name = "/", 0};
    success = fwrite(&root, sizeof(fat_item), 1, fs->file) == 1;
    if (!success) return ERR_FAT | ERR_WRITE;

    printf("--------------------------------------\n");

    // reopen file in mode for reading
    if (fs->file) fclose(fs->file);
    fs->file = fopen(fs->filepath, "r+b");
    if (!fs->file) return ERR_FAT | ERR_OPEN;

    fprintf(stdout, "\ndone\n");

    return SUCCESS;
}
