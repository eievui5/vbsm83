// C FILE escapsulation class.
#pragma once

#include <stdarg.h>
#include <stdio.h>

class File {
public:
	FILE* file;

	~File() {fclose(file);}

	operator FILE*() {return file;}

	inline bool eof() {return feof(file);}
	inline char getc() {return fgetc(file);}
	inline void open(char* path, const char* flags) {file = fopen(path, flags);}

	inline void printf(char* fmt, ...) {
		va_list ap;

		va_start(ap, fmt);
		vfprintf(file, fmt, ap);
		va_end(ap);
	}
};
