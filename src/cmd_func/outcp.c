#include "types.h"
#include "utils.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

rc cmd_func_outcp(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc != 3) return ERR_ARGC;
    if (!fs) return ERR_INTERN;
    if (!fs->file) return ERR_FAT | ERR_NOEXISTS;

    // variables
    bool success        = false;
    FILE *file          = 0;
    size_t file_size    = 0;
    size_t n_write      = 0;
    uint32_t n_clusters = 0;
    uint32_t cls        = 0;
    uint32_t i          = 0;
    uint32_t clusters[fs->header.cluster_count];
    char buffer[fs->header.cluster_size];
    file_item vItem     = {0};

    int32_t n_path  = 0;
	rc return_code  = SUCCESS;
    char path[MAX_PATH_NUM][BUFFER_SIZE] = {0};

	// traverse path (n - 1)
    cls = *cluster;
    n_path = parse_path(argv[1], path);
	return_code = traverse(fs, &cls, path, n_path - 1);
	if (return_code != SUCCESS) return return_code;

    // find file by name
    fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size, SEEK_SET);
    for (i = 0; i < fs->header.cluster_size; i += sizeof(file_item)) {
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;          // could an entry be read
        if (!vItem.name[0]) continue;                     // does entry have a name
        if (strcmp(path[n_path - 1], vItem.name)) continue;        // does entry name match
        if (vItem.size == FAT_DIRECTORY) return ERR_ARGV; // is entry a directory
        cls = vItem.start_cluster;
        file_size = vItem.size;
        n_write = file_size;
        break;
    } if (i >= fs->header.cluster_size) return ERR_ARGV;

    // traverse file clusters
    i = 0;
    do {
        clusters[i++] = cls;
        fseek(fs->file, fs->header.fat_start_address + cls * sizeof(uint32_t), SEEK_SET);
        success = fread(&cls, sizeof(uint32_t), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;
    } while (cls != FAT_FILE_END);
    n_clusters = i;

    // create or clear output file
    file = fopen(argv[2], "w");
    if (!file) return ERR_FILE | ERR_OPEN;

    // reopen output file to append
    fclose(file);
    file = fopen(argv[2], "a");
    if (!file) return ERR_FILE | ERR_OPEN;

    // write data clusters to output file
    for (i = 0; i < n_clusters; i++) {
        memset(buffer, 0, fs->header.cluster_size);
        fseek(fs->file, fs->header.data_start_address + clusters[i] * fs->header.cluster_size, SEEK_SET);
        success = fread(buffer, fs->header.cluster_size, 1, fs->file) == 1;
        if (!success) {
            // close output file
            fclose(file);
            return ERR_FAT | ERR_READ;
        }

        success = fwrite(buffer, n_write >= fs->header.cluster_size ? fs->header.cluster_size : n_write, 1, file) == 1;
        n_write -= fs->header.cluster_size;
        if (!success) {
            // close output file
            fclose(file);
            return ERR_FILE | ERR_WRITE;
        }
    }

    // close output file
    fclose(file);
    printf("done\n");
    return SUCCESS;
}
