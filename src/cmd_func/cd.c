#include "types.h"
#include "utils.h"

#include <string.h>

rc cmd_func_cd(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc > 2) return ERR_ARGC;
    if (!fs) return ERR_INTERN;
    if (!fs->file) return ERR_INTERN;

    char *path = argc == 1 ? "/" : argv[1];

    // change directory to root
    if (!strcmp(path, "/")) {
        *cluster = 0;
        return SUCCESS;
    }

    char path_arr[MAX_PATH_NUM][BUFFER_SIZE] = {0};
    int i = parse_path(path, path_arr);
    uint32_t cls = *cluster;

    if (path_arr[0][0] == '/') cls = 0;
	rc rc = traverse(fs, &cls, path_arr, i);
	if (rc != SUCCESS) return rc;

    *cluster = cls;
    return SUCCESS;
}
