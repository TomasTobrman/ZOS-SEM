#include "fat.h"
#include "types.h"
#include "utils.h"

rc cmd_func_mkdir(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc != 2) return ERR_ARGC;
    if (!fs) return ERR_INTERN;

    uint32_t cls    = 0;
    int32_t n_path  = 0;
	rc return_code  = SUCCESS;
    char path[MAX_PATH_NUM][BUFFER_SIZE] = {0};

	// traverse path (n - 1)
    cls = *cluster;
    n_path = parse_path(argv[1], path);
	return_code = traverse(fs, &cls, path, n_path - 1);
	if (return_code != SUCCESS) return return_code;

    return fat_mkdir(fs, cls, path[n_path - 1]);
}
