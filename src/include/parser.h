#pragma once

#include <stdio.h>

#include "statements.h"

extern const char* OPERATOR[];
extern const char* STORAGE_CLASS[];
extern const char* TYPE[];

Declaration** fparse_textual_ir(FILE* infile);
