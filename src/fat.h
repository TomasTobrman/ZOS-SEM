#pragma once

#include "types.h"

rc fat_open(fat **fs, const char *fp);

rc fat_close(fat **fs);

rc fat_format(fat *fs, uint32_t size);
