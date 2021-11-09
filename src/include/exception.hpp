#pragma once

#include <stdio.h>

extern bool enable_info;
extern unsigned error_count;

void warn(char const *fmt, ...);
void error(char const *fmt, ...);
void fatal(char const *fmt, ...);