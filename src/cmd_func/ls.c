#include "types.h"
#include "utils.h"

rc cmd_func_ls(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc > 3) return ERR_ARGC;
    if (!fs) return ERR_INTERN;
    if (!fs->file) return ERR_FAT | ERR_NOEXISTS;

    // variables
    bool show_hidden = false;
    bool show_list   = false;
    bool success     = false;
    uint32_t cls     = 0;
    uint32_t i       = 0;
    uint32_t j       = 0;
    file_item vItem  = {0};

    int32_t n_path  = 0;
	char *path_str  = argc > 1 ? argv[argc - 1] : ".";
	rc return_code  = SUCCESS;
    char path[MAX_PATH_NUM][BUFFER_SIZE] = {0};

    // parse arguments
    if (argc == 1) {
    } else if (argv[1][0] == '-') {
        while (*(++(argv[1]))) {
            switch (*(argv[1])) {
                case 'a': show_hidden = true; break;
                case 'l': show_list = true; break;
                default: return ERR_ARGV;
            }
        }

		if (argc == 2) path_str = ".";
    }

	// traverse path
    cls = *cluster;
    n_path = parse_path(path_str, path);
	return_code = traverse(fs, &cls, path, n_path);
	if (return_code != SUCCESS) return return_code;

    fseek(fs->file, fs->header.data_start_address + cls * fs->header.cluster_size, SEEK_SET);
    for (i = 0; i < fs->header.cluster_size; i += sizeof(file_item)) {
        success = fread(&vItem, sizeof(file_item), 1, fs->file) == 1;
        if (!success) return ERR_FAT | ERR_READ;
        if (!vItem.name[0]) continue;
        if (!show_hidden && vItem.name[0] == '.') continue;

        bool is_dir = vItem.size == FAT_DIRECTORY;

        j++;

        if (show_list) {
            printf("%c  %9u  %s%s%s\n", is_dir ? 'd' : '.', is_dir ? (uint32_t)sizeof(file_item) : vItem.size, is_dir ? "\033[1;34m" : "", vItem.name, "\033[0m");
        } else {
            printf("%s%s%s  ", vItem.size == FAT_DIRECTORY ? "\033[1;34m" : "", vItem.name, "\033[0m");
        }
    }
    if (j && !show_list) printf("\n");

    return SUCCESS;
}
