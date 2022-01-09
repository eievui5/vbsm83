#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "statements.h"

extern const char* OPERATOR[];
extern const char* STORAGE_CLASS[];
extern const char* TYPE[];

Declaration** fparse_textual_ir(FILE* infile);

static inline bool strequ(const char* s1, const char* s2) {
    return strcmp(s1, s2) == 0;
}
