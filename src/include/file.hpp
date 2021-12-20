// C FILE escapsulation class.
#pragma once

#include <stdarg.h>
#include <stdio.h>

class File {
  public:
    FILE* file;

    ~File() { fclose(file); }

    operator FILE*() { return file; }

    bool eof() { return feof(file); }
    char getc() { return fgetc(file); }
    void open(char* path, const char* flags) { file = fopen(path, flags); }

    void printf(char* fmt, ...) {
        va_list ap;

        va_start(ap, fmt);
        vfprintf(file, fmt, ap);
        va_end(ap);
    }
};
