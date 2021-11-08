#pragma once

#include <stdio.h>

extern bool enable_info;
extern unsigned error_count;

void error(char const *fmt, ...);