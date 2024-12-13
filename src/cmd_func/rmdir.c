#include "types.h"
#include "utils.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

rc cmd_func_rmdir(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc != 2) return ERR_ARGC;
    if (!fs) return ERR_INTERN;

    bool success     = false;
    bool found       = false;
    uint32_t i       = 0;
    uint32_t j       = 0;
    file_item vItem  = {0};
    uint32_t cls     = 0;
	uint32_t cls_par = 0;
	char buffer[fs->header.cluster_size];
	memset(buffer, 0, fs->header.cluster_size);

    int32_t n_path   = 0;
	rc return_code   = SUCCESS;
    char path[MAX_PATH_NUM][BUFFER_SIZE] = {0};

	// traverse path (n - 1)
    cls = *cluster;
    n_path = parse_path(argv[1], path);
	return_code = traverse(fs, &cls, path, n_path - 1);
	if (return_code != SUCCESS) return return_code;
	cls_par = cls;

    // cannot remove "/" or "." or ".." directories
    if (!strcmp(argv[1], "/")) return ERR_ARGV;
    if (!strcmp(path[n_path - 1], ".")) return ERR_ARGV;
    if (!strcmp(path[n_path - 1], "..")) return ERR_ARGV;

    // find directory by name
    fseek(fs->file, fs->header.data_start_address + cls_par * fs->header.cluster_size, SEEK_SET);
    for (i = 0; i < fs->header.cluster_size; i += sizeof(file_item)) {
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;

        if (!vItem.name[0]) continue;
        if (strcmp(vItem.name, path[n_path - 1])) continue;
        if (vItem.size != FAT_DIRECTORY) return ERR_ARGV;

        cls = vItem.start_cluster;

        found = true;
        break;
    }
    if (!found) return ERR_ARGV;

    // check if dir empty
    fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size + 2 * sizeof(file_item) /* skip "." and ".." */, SEEK_SET);
    for (j = 2 * sizeof(file_item); j < fs->header.cluster_size; j += sizeof(file_item)) {
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;
        if (vItem.name[0]) return ERR_NOEMPTY;
    }

	// remove data
    fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size, SEEK_SET);
    success = fwrite(&buffer, fs->header.cluster_size, 1, fs->file) == 1;
    if (!success) return ERR_FAT | ERR_WRITE;

    // remove data entry
    file_item item_empty = {0};
    fseek(fs->file, fs->header.data_start_address + cls_par * fs->header.cluster_size + i, SEEK_SET);
    success = fwrite(&item_empty, sizeof(file_item), 1, fs->file) == 1;
    if (!success) return ERR_FAT | ERR_WRITE;

    // remove table entry
    fseek(fs->file, fs->header.fat_start_address + cls * sizeof(uint32_t), SEEK_SET);
    uint32_t val_unused = FAT_UNUSED;
    success = fwrite(&val_unused, sizeof(uint32_t), 1, fs->file) == 1;
    if (!success) return ERR_FAT | ERR_WRITE;

    return SUCCESS;
}
