#include "types.h"
#include "utils.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

rc cmd_func_defrag(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
	if (argc != 2) return ERR_ARGC;
	if (!fs) return ERR_INTERN;
	if (!fs->file) return ERR_INTERN;

	// variables
	bool success           = false;
	uint32_t cls           = 0;
	uint32_t cls_path      = 0;
	uint32_t i             = 0;
	uint32_t j             = 0;
	file_item vItem        = {0};
	uint32_t n_clusters    = 0;
	uint32_t clusters[fs->header.cluster_count];
	char buffer[fs->header.cluster_size];

	int32_t n_path  = 0;
	rc return_code  = SUCCESS;
	char path[MAX_PATH_NUM][BUFFER_SIZE] = {0};

	// traverse path (n - 1)
	cls_path = *cluster;
	n_path = parse_path(argv[1], path);
	return_code = traverse(fs, &cls_path, path, n_path - 1);
	if (return_code != SUCCESS) return return_code;

	// find file entry
	fseek(fs->file, fs->header.data_start_address + cls_path * fs->header.cluster_size, SEEK_SET);
	for (i = 0; i < fs->header.cluster_size; i += sizeof(file_item)) {
		success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
		if (!success) return ERR_FAT | ERR_READ;
		if (!vItem.name[0]) continue;
		if (strcmp(vItem.name, path[n_path - 1])) continue;
		cls = vItem.start_cluster;
		break;
	} if (i >= fs->header.cluster_size) return ERR_ARGV;

	// get clusters
	do {
		clusters[n_clusters++] = cls;
		fseek(fs->file, fs->header.fat_start_address + cls * sizeof(uint32_t), SEEK_SET);
		success = fread(&cls, sizeof(uint32_t), 1, fs->file) == 1;
		if (!success) return ERR_FAT | ERR_READ;
	} while (cls != FAT_FILE_END);

	// find empty space
	for (vItem.start_cluster = 0; vItem.start_cluster < fs->header.cluster_count; vItem.start_cluster++) {
		fseek(fs->file, fs->header.fat_start_address + vItem.start_cluster * sizeof(uint32_t), SEEK_SET);
		for (j = 0; j < n_clusters; j++) {
			success = fread(&cls, sizeof(uint32_t), 1, fs->file) == 1;
			if (!success) return ERR_FAT | ERR_READ;
			if (cls != FAT_UNUSED) break; // did not find space
		} if (j >= n_clusters) break; // found space
	} if (vItem.start_cluster >= fs->header.cluster_count) return ERR_FAT | ERR_FULL; // did not find space

	// move data
	for (j = 0; j < n_clusters; j++) {
		// read original cluster data
		fseek(fs->file, fs->header.data_start_address + clusters[j] * fs->header.cluster_size, SEEK_SET);
		success = fread(&buffer, fs->header.cluster_size, 1, fs->file) == 1;
		if (!success) return ERR_FAT | ERR_READ;

		// write new cluster data
		fseek(fs->file, fs->header.data_start_address + (vItem.start_cluster + j) * fs->header.cluster_size, SEEK_SET);
		success = fwrite(&buffer, fs->header.cluster_size, 1, fs->file) == 1;
		if (!success) return ERR_FAT | ERR_WRITE;

		// remove original cluster data
		memset(buffer, 0, fs->header.cluster_size);
		fseek(fs->file, fs->header.data_start_address + clusters[j] * fs->header.cluster_size, SEEK_SET);
		success = fwrite(&buffer, fs->header.cluster_size, 1, fs->file) == 1;
		if (!success) return ERR_FAT | ERR_WRITE;

		// write new cluster index
		fseek(fs->file, fs->header.fat_start_address + (vItem.start_cluster + j) * sizeof(uint32_t), SEEK_SET);
		cls = j + 1 < n_clusters ? vItem.start_cluster + j + 1 : FAT_FILE_END;
		success = fwrite(&cls, sizeof(uint32_t), 1, fs->file) == 1;
		if (!success) return ERR_FAT | ERR_WRITE;

		// remove original cluster index
		fseek(fs->file, fs->header.fat_start_address + clusters[j] * sizeof(uint32_t), SEEK_SET);
		cls = FAT_UNUSED;
		success = fwrite(&cls, sizeof(uint32_t), 1, fs->file) == 1;
		if (!success) return ERR_FAT | ERR_WRITE;
	}

	// rewrite start cluster
	fseek(fs->file, fs->header.data_start_address + cls_path * fs->header.cluster_size + i, SEEK_SET);
	success = fwrite(&vItem, sizeof(file_item), 1, fs->file) == 1;
	if (!success) return ERR_FAT | ERR_WRITE;

	return SUCCESS;
}
