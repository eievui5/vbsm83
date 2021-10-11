#pragma once

#include <stdbool.h>

extern bool enable_info;
extern unsigned error_count;

void error(char const *fmt, ...);
void info(FILE* output, char const *fmt, ...);