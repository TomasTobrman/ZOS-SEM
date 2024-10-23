#include "types.h"
#include "fat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

rc fat_open(file_system **fs, const char *fp) {
    if (!fs) return ERR_INTERN;

    *fs = (file_system*)malloc(sizeof(file_system));
    if (!*fs) return ERR_INTERN | ERR_MEMORY;

    memset(*fs, 0, sizeof(file_system));
    (*fs)->filepath = fp;
    (*fs)->file = fopen((*fs)->filepath, "r+b");
    if (!(*fs)->file) return ERR_FAT | ERR_OPEN;

    rewind((*fs)->file);
    bool success = fread(&(*fs)->header, sizeof(header), 1, (*fs)->file) == 1;
    if (!success) return ERR_FAT | ERR_READ;

    return SUCCESS;
}

rc fat_close(file_system **fs) {
    if (!fs || !*fs) return ERR_INTERN;

    if ((*fs)->file) {
        fclose((*fs)->file);
        (*fs)->file = NULL;
    }

    free(*fs);
    *fs = NULL;

    return SUCCESS;
}

rc fat_format(file_system *fs, uint32_t size) {
    if (!fs) return ERR_INTERN;

    // variables
    bool success = false;
    uint32_t i   = 0;

    uint8_t  v8  = 0;
    uint32_t v32 = 0;

    // header
    strcpy(fs->header.signature, "ttobrman");
    fs->header.disk_size          = size;
    fs->header.cluster_size       = KILOBYTE;
    fs->header.cluster_count      = (size - sizeof(header)) / fs->header.cluster_size;
    fs->header.fat_start_address  = sizeof(header);
    fs->header.data_start_address = fs->header.fat_start_address + sizeof(uint32_t) * fs->header.cluster_count;

    // write zeros into file
    printf("---- FORMATTING ----------------------\n");
    printf(" [%u B]\n", fs->header.disk_size);

    if (fs->file) fclose(fs->file);
    fs->file = fopen(fs->filepath, "w+b");
    if (!fs->file) return ERR_FAT | ERR_OPEN;

    rewind(fs->file);
    for (i = 0, v8 = 0; i < fs->header.disk_size; i++) {
        success = fwrite(&v8, 1, 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_WRITE;
    }

    // reopen file in mode for reading
    if (fs->file) fclose(fs->file);
    fs->file = fopen(fs->filepath, "r+b");
    if (!fs->file) return ERR_FAT | ERR_OPEN;
    rewind(fs->file);

    // initial data
    printf("---- HEADER --------------------------\n");
    printf("  signature          : %s\n", fs->header.signature);
    printf("  disk size          : %d\n", fs->header.disk_size);
    printf("  cluster size       : %d\n", fs->header.cluster_size);
    printf("  cluster count      : %d\n", fs->header.cluster_count);
    printf("  fat start address  : %d\n", fs->header.fat_start_address);
    printf("  data start address : %d\n", fs->header.data_start_address);
    printf("--------------------------------------\n");

    success = fwrite(&fs->header, sizeof(header), 1, fs->file) == 1;
    if (!success) return ERR_FAT | ERR_WRITE;

    fseek(fs->file, fs->header.fat_start_address, SEEK_SET);
    for (i = 0, v32 = FAT_UNUSED; i < fs->header.cluster_count; i++) {
        success = fwrite(&v32, sizeof(uint32_t), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_WRITE;
    }

    // root
    printf(" creating root directory...\n");

    rc rc = fat_mkdir(fs, 0, "/");
    if (rc != SUCCESS) return rc;

    printf("--------------------------------------\n");

    fprintf(stdout, "\ndone\n");

    return SUCCESS;
}

rc fat_mkdir(file_system *fs, uint32_t parent, const char *name) {
    if (!fs) return ERR_INTERN;
    if (!fs->file) return ERR_INTERN;

    // variables
    bool success    = false;

    uint32_t i      = 0;
    uint32_t j      = 0;

    uint32_t v32    = 0;
    file_item vItem = {0};

    // find unused cluster
    fseek(fs->file, fs->header.fat_start_address, SEEK_SET);
    for (i = 0; i < fs->header.cluster_count; i++) {
        success = fread(&v32, sizeof(uint32_t), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;
        if (v32 == FAT_UNUSED) break;
    }
    if (i >= fs->header.cluster_count) return ERR_FAT | ERR_FULL;

    // find empty space in data block in parent cluster
    fseek(fs->file, fs->header.data_start_address + parent * fs->header.cluster_size, SEEK_SET);
    for (j = 0; j < fs->header.cluster_size; j += sizeof(file_item)) {
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;
        if (!vItem.name[0]) break;
    }
    if (j >= fs->header.cluster_size) return ERR_FAT | ERR_FULL;

    // items
    file_item cur = {0};
    file_item par = {0};
    file_item dir = {0};

    strcpy(cur.name, ".");
    strcpy(par.name, "..");
    strcpy(dir.name, name);

    cur.size = FAT_DIRECTORY;
    par.size = FAT_DIRECTORY;
    dir.size = FAT_DIRECTORY;

    cur.start_cluster = i;
    par.start_cluster = parent;
    dir.start_cluster = i;

    // write folder to fat table
    fseek(fs->file, fs->header.fat_start_address + i * sizeof(uint32_t), SEEK_SET);
    v32 = FAT_FILE_END;
    success = fwrite(&v32, sizeof(uint32_t), 1, fs->file) == 1;
    if (!success) return ERR_FAT | ERR_WRITE;

    // write current and parent folders to new directory
    fseek(fs->file, fs->header.data_start_address + i * fs->header.cluster_size, SEEK_SET);
    success = fwrite(&cur, sizeof(file_item), 1, fs->file) == 1;
    if (!success) return ERR_FAT | ERR_WRITE;
    success = fwrite(&par, sizeof(file_item), 1, fs->file) == 1;
    if (!success) return ERR_FAT | ERR_WRITE;

    // end early if creating root dir
    if (i == 0) return SUCCESS;

    // write new directory to parent cluster
    fseek(fs->file, fs->header.data_start_address + parent * fs->header.cluster_size + j, SEEK_SET);
    success = fwrite(&dir, sizeof(file_item), 1, fs->file) == 1;
    if (!success) return ERR_FAT | ERR_WRITE;

    return SUCCESS;
}
