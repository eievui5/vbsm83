#pragma once

#include <stdio.h>

extern bool enable_info;
extern unsigned error_count;

void info(char const* fmt, ...);
void warn(char const* fmt, ...);
void error(char const* fmt, ...);
void fatal(char const* fmt, ...);