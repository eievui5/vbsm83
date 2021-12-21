#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

bool enable_info = false;
unsigned error_count = 0;

void warn(char const* fmt, ...) {
    va_list ap;

    fputs("\033[1m\033[95mwarn: \033[0m", stderr);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    putc('\n', stderr);
}

void error(char const* fmt, ...) {
    va_list ap;

    fputs("\033[1m\033[31merror: \033[0m", stderr);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    putc('\n', stderr);

    error_count++;
}

void fatal(char const* fmt, ...) {
    va_list ap;

    fputs("\033[1m\033[31mfatal: \033[0m", stderr);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    putc('\n', stderr);

    exit(1);
}
