#include "../fat.h"
#include "../types.h"

#include <stdint.h>

rc cmd_func_format(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc != 2) return ERR_ARGC;
    if (!fs) return ERR_INTERN;

    char *s = argv[1];
    uint32_t size = 0;
    for (; s; s++) {
        if (*s == '\0') {
            break;
        } else if (*s == 'B' && *(s + 1) == '\0') {
            break;
        } else if (*s == 'k' && *(s + 1) == 'B') {
            size *= KILOBYTE;
        } else if (*s == 'M' && *(s + 1) == 'B') {
            size *= MEGABYTE;
        } else if (*s == 'G' && *(s + 1) == 'B') {
            size *= GIGABYTE;
        } else if (*s >= '0' && *s <= '9') {
            size *= 10;
            size += *s - '0';
        } else {
            return ERR_ARGV;
        }
    }
    if (size < (2 * KILOBYTE) || size > GIGABYTE) return ERR_ARGV;

    rc rc = fat_format(fs, size);
    if (rc != SUCCESS) return rc;

    return SUCCESS;
}
