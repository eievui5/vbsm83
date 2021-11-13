#pragma once

#include <iostream>
#include <stdio.h>

extern bool enable_info;
extern unsigned error_count;

// printf-style debug output.
void info(char const* fmt, ...);
void warn(char const* fmt, ...);
void error(char const* fmt, ...);
void fatal(char const* fmt, ...);