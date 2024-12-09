#pragma once

#include "types.h"

rc fat_open(file_system **fs, const char *fp);
rc fat_close(file_system **fs);
rc fat_format(file_system *fs, uint32_t size);
rc fat_mkdir(file_system *fs, uint32_t parent, const char *name);
rc fat_cwd(file_system *fs, uint32_t *cluster, char *buffer);
