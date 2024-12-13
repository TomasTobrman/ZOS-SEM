#include "types.h"
#include "utils.h"

#include <string.h>

rc cmd_func_cp(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc != 3) return ERR_ARGC;
    if (!fs) return ERR_INTERN;
    if (!fs->file) return ERR_FAT | ERR_NOEXISTS;

    // variables
    bool success          = false;
    uint32_t cls          = 0;
    uint32_t cls_next     = 0;
    uint32_t i            = 0;
    uint32_t j            = 0;
    uint32_t file_size    = 0;
    file_item vItem       = {0};
    uint32_t n_clusters   = 0;
    uint32_t clusters[fs->header.cluster_count];
    char buffer[fs->header.cluster_size];

    int32_t n_path        = 0;
	rc return_code        = SUCCESS;
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
        if (!success) return ERR_FAT | ERR_READ;

        if (!vItem.name[0]) continue;
        if (strcmp(vItem.name, path[n_path - 1])) continue;
        if (vItem.size == FAT_DIRECTORY) return ERR_ARGV;

        file_size = vItem.size;
        cls_next = vItem.start_cluster;
        break;
    } if (i >= fs->header.cluster_size) return ERR_ARGV;

    // calculate number of clusters needed
    n_clusters = file_size / fs->header.cluster_size + (file_size % fs->header.cluster_size ? 1 : 0);

    // check if file fits and save available clusters
    fseek(fs->file, fs->header.fat_start_address, SEEK_SET);
    for (i = 0; i < fs->header.cluster_count; i++) {
        success = fread(&cls, sizeof(uint32_t), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;
        if (cls == FAT_UNUSED) clusters[j++] = i;
    } if (j < n_clusters) return ERR_FAT | ERR_FULL;

    // set last cluster to file end mark
    clusters[n_clusters] = FAT_FILE_END;

    // restore in file start cluster
    cls = cls_next;

    // traverse clusters and copy data
    j = 0;
    while (cls != FAT_FILE_END && j < n_clusters) {
        // get next in cluster index
        fseek(fs->file, fs->header.fat_start_address + cls * sizeof(uint32_t), SEEK_SET);
        success = fread(&cls_next, sizeof(uint32_t), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;

        // write next out cluster index
        fseek(fs->file, fs->header.fat_start_address + clusters[j] * sizeof(uint32_t), SEEK_SET);
        success = fwrite(&clusters[j + 1], sizeof(uint32_t), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_WRITE;

        // get in data cluster
        fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size, SEEK_SET);
        success = fread(buffer, fs->header.cluster_size, 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;

        // write out data cluster
        fseek(fs->file, fs->header.data_start_address + clusters[j] * fs->header.cluster_size, SEEK_SET);
        success = fwrite(buffer, fs->header.cluster_size, 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_WRITE;

        // go to next cluster
        cls = cls_next;
        j++;
    }

	// traverse path (n - 1)
	cls = *cluster;
	n_path = parse_path(argv[2], path);
	return_code = traverse(fs, &cls, path, n_path - 1);
	if (return_code != SUCCESS) return return_code;

    // create new out file entry
    fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size, SEEK_SET);
    for (i = 0; i < fs->header.cluster_size; i += sizeof(file_item)) {
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;
        if (vItem.name[0]) continue;

        strcpy(vItem.name, path[n_path - 1]);
        vItem.size = file_size;
        vItem.start_cluster = clusters[0];

        fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size + i, SEEK_SET);
        success = fwrite(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_WRITE;
        break;
    } if (i >= fs->header.cluster_size) return ERR_ARGV;

    return SUCCESS;
}
