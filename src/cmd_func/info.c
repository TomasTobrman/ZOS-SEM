#include "types.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

rc cmd_func_info(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc != 2) return ERR_ARGC;
    if (!fs) return ERR_INTERN;
    if (!fs->file) return ERR_FAT | ERR_NOEXISTS;

    // variables
    bool success    = false;
    bool is_dir     = false;
    uint32_t i      = 0;
    uint32_t cls    = 0;
    file_item vItem = {0};

    int32_t n_path  = 0;
	rc return_code  = SUCCESS;
    char path[MAX_PATH_NUM][BUFFER_SIZE] = {0};

	// traverse path (n - 1)
    cls = *cluster;
    n_path = parse_path(argv[1], path);
	return_code = traverse(fs, &cls, path, n_path - 1);
	if (return_code != SUCCESS) return return_code;

    // find entry
    fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size, SEEK_SET);
    for (i = 0; i < fs->header.cluster_size; i += sizeof(file_item)) {
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;

        if (!vItem.name[0]) continue;
        if (strcmp(vItem.name, path[n_path - 1])) continue;

        cls = vItem.start_cluster;
        break;
    } if (i >= fs->header.cluster_size) return ERR_ARGV;

    is_dir = vItem.size == FAT_DIRECTORY;

    printf("  name:       %s\n", vItem.name);
    printf("  directory:  %s\n", is_dir ? "true" : "false");
    if (!is_dir)
        printf("  size:       %u\n", vItem.size);

    // traverse clusters
    printf("  clusters:   %d", cls);
    while (cls != FAT_FILE_END) {
        // get next cluster
        fseek(fs->file, fs->header.fat_start_address + cls * sizeof(uint32_t), SEEK_SET);
        success = fread(&cls, sizeof(uint32_t), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;
        if (cls == FAT_FILE_END) break;
        printf(", %d", cls);
    } printf("\n");

    return SUCCESS;
}
