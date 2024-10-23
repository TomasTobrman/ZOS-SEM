#include "../fat.h"
#include "../types.h"

rc cmd_func_ls(file_system *fs, uint32_t *cluster, uint8_t argc, char **argv) {
    if (argc > 3) return ERR_ARGC;
    if (!fs) return ERR_INTERN;

    // TODO: check path validity
    char *path = argc > 1 ? argv[argc - 1] : ".";
    bool show_hidden = false;
    bool show_list = false;

    if (argc == 1) {
    } else if (argv[1][0] == '-') {
        while (*(++(argv[1]))) {
            switch (*(argv[1])) {
                case 'a': show_hidden = true; break;
                case 'l': show_list = true; break;
                default: return ERR_ARGV;
            }
        }

        if (argc == 2) path = ".";
    }

    return fat_ls(fs, *cluster, path, show_hidden, show_list);
}
