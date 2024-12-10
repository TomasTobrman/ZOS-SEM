#pragma once

#include "types.h"

void report_rc(const char *reporter, rc rc);

/** @author Adam Rosenfield, Dave Gray (StackOverflow) */
char *trimwhitespace(char *str);

/** @author Eli Bendersky, Toby Speight (StackOverflow) */
void prepend(char *s, const char *t);

int parse_path(char *path, char result[32][BUFFER_SIZE]);

rc parse_command(file_system *fs, uint32_t *cluster, char *buffer);
