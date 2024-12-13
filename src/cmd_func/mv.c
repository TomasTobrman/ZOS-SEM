#include "types.h"
#include "utils.h"

#include <stdio.h>
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
	char out_fn[FAT_ITEM_NAME_SIZE] = {0};

    int32_t n_path        = 0;
	rc return_code        = SUCCESS;
    char path[MAX_PATH_NUM][BUFFER_SIZE] = {0};


	if (!strcmp(argv[2], "/")) {
		cls = 0;
		cls_out = 0;
	} else {
		// traverse path (n - 1)
		cls = *cluster;
		n_path = parse_path(argv[2], path);
		memcpy(out_fn, path[n_path - 1], strlen(path[n_path - 1]));
		return_code = traverse(fs, &cls, path, n_path - 1);
		if (return_code != SUCCESS) return return_code;
		cls_out = cls;

		// find out entry by name
		fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size, SEEK_SET);
		for (i = 0; i < fs->header.cluster_size; i += sizeof(file_item)) {
			success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
			if (!success) return ERR_FAT | ERR_READ;
			if (!vItem.name[0]) continue;
			if (strcmp(vItem.name, path[n_path - 1])) continue;
			if (vItem.size != FAT_DIRECTORY) return ERR_ARGV;
			cls_out = vItem.start_cluster;
			break;
		} if (i >= fs->header.cluster_size) rename = true;
	}

	// traverse path (n - 1)
    cls = *cluster;
	memset(path, 0, MAX_PATH_NUM * BUFFER_SIZE);
    n_path = parse_path(argv[1], path);
	return_code = traverse(fs, &cls, path, n_path - 1);
	if (return_code != SUCCESS) return return_code;

    // find in entry by name
    fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size, SEEK_SET);
    for (i = 0; i < fs->header.cluster_size; i += sizeof(file_item)) {
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;
        if (!vItem.name[0]) continue;
        if (strcmp(vItem.name, path[n_path - 1])) continue;
        break;
    } if (i >= fs->header.cluster_size) return ERR_ARGV;

    // rename entry
    if (rename) {
        fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size + i, SEEK_SET);
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;

		memset(vItem.name, 0, FAT_ITEM_NAME_SIZE);
		memcpy(vItem.name, out_fn, strlen(out_fn) <= FAT_ITEM_NAME_SIZE ? strlen(out_fn) : FAT_ITEM_NAME_SIZE);

        fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size + i, SEEK_SET);
        success = fwrite(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_WRITE;
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
