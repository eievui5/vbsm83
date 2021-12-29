#pragma once

#include <stdbool.h>

/* Print a warning message to stderr.
 * Warnings alert the user to strange or unsafe behavior that does not prevent
 * execution.
 */
void warn(char const* fmt, ...);

/* Print an error message to stderr.
 * Errors are for exceptions which should halt the program execution but are not
 * immediately fatal. This allows the program to continue running to find and
 * alert the user to other errors.
 */
void error(char const* fmt, ...);

/* Print a fatal error message to stderr and end execution immediately.
 */
void fatal(char const* fmt, ...);

/* Exit the program if any errors have occured.
 */
void errcheck();
