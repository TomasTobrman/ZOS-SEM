#include "types.h"
#include "utils.h"

#include <stdio.h>

rc cmd_func_load(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    // variables
    rc  return_code          = 0;
    FILE *file               = 0;
    size_t n_alloc           = 0;
    char *buffer             = 0;

    // check if file exists
    file = fopen(argv[1], "r");
    if (!file) return ERR_FILE | ERR_OPEN;

    // get line from input file
    while(getline(&buffer, &n_alloc, file) != -1) {
        printf("$ %s", buffer);
        return_code = parse_command(fs, cluster, buffer);
        if (return_code != SUCCESS) return return_code;
    }

    return SUCCESS;
}
