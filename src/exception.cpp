#include <stdarg.h>
#include <stdio.h>

bool enable_info = false;
unsigned error_count = 0;

/* Print an error message to stderr
 * This is for exceptions which should halt the program execution but are not
 * immediately fatal. This allows the program to continue running to find and
 * alert the user to other errors.
*/
void error(char const *fmt, ...) {
    va_list ap;

    fputs("error: ", stderr);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    putc('\n', stderr);

    error_count++;
}