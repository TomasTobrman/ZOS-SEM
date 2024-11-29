#include "utils.h"
#include "types.h"

#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

void report_rc(const char *reporter, rc rc) {
    if (rc & ERR_ARGC)
        fprintf(stderr, "%s: wrong number of arguments.\n  try running the \"help\" command.\n", reporter);
    if (rc & ERR_ARGV)
        fprintf(stderr, "%s: invalid arguments.\n  try running the \"help\" command.\n", reporter);

    if (rc & ERR_READ)
        fprintf(stderr, "%s: could not read from file.\n", reporter);
    if (rc & ERR_WRITE)
        fprintf(stderr, "%s: could not write into file.\n", reporter);

    if (rc & ERR_NOEMPTY)
        fprintf(stderr, "%s: directory is not empty.\n", reporter);

    if (rc & ERR_CMD && rc & ERR_NOEXISTS)
        fprintf(stderr, "%s: No such file or directory\n", reporter);

    if (rc & ERR_FAT && rc & ERR_CREATE)
        fprintf(stderr, "%s: could not create DAT file.\n", reporter);
    if (rc & ERR_FAT && rc & ERR_OPEN)
        fprintf(stderr, "%s: could not open DAT file.\n  try running the \"format\" command to create it.\n", reporter);
    if (rc & ERR_FAT && rc & ERR_FULL)
        fprintf(stderr, "%s: file-system is full.\n", reporter);

    if (rc & ERR_INTERN && rc & ERR_MEMORY)
        fprintf(stderr, "%s: internal memory error.\n", reporter);
}

char *trimwhitespace(char *str) {
    char *end;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

void prepend(char *s, const char *t) {
    size_t len = strlen(t);
    memmove(s + len, s, strlen(s) + 1);
    memcpy(s, t, len);
}

int parse_path(char *path, char result[MAX_PATH_NUM][BUFFER_SIZE]) {
    int i = 0, j = 0;
    char *str = 0;

    str = strtok(path, "/");
    while (str != 0 && i < 32) {
        if (!str[0]) continue; // empty
        int len = strlen(str);
        for (j = 0; j < len && j < BUFFER_SIZE; j++)
            result[i][j] = str[j];
        i++;
        str = strtok(0, "/");
    }

    return i;
}
