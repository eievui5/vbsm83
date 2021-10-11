#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

static struct option const longopts[] = {
    {"input",	required_argument,	NULL, 'i'},
    {"output",	required_argument,	NULL, 'o'},
    {NULL,		no_argument,		NULL, 0}
};
static const char shortopts[] = "i:o:";
static const char* token_types[] = {
    "Unknown",
    "Comment",
    "Operator",
    "Type",
    "Identifier",
};

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

int main(int argc, char* argv[]) {
    FILE* input_file = NULL;
    FILE* output_file = NULL;
    char option_char;

    // Parse command-line options.
    while ((option_char = getopt_long_only(argc, argv, shortopts, longopts, NULL)) != -1) {
        switch (option_char) {
            case 'i':
                input_file = fopen(optarg, "r");
                if (input_file == NULL) {
                    error("Error opening file %s!", *optarg);
                }
                break;
            case 'o':
                output_file = fopen(optarg, "w");
                if (output_file == NULL) {
                    error("Error opening file %s!", *optarg);
                }
                break;
        }
    }

    // Verify inputs.
    if (input_file == NULL) {
        error("Missing input file!");
    }
    if (output_file == NULL) {
        error("Missing output file!");
    }

    // Check for errors.
    if (error_count > 0) {
        fprintf(stderr, "Failed with %u errors.\n", error_count);
        return 1;
    }

    // Now get to the parsing!
    int entries = 0;
    while (1) {
        struct token* next_tok = get_token(input_file);

        if (next_tok == NULL) {
            break;
        }

        printf("%-12s : %-12s, ", token_types[next_tok->type], next_tok->string);
        free_token(next_tok);
        entries++;
        if (entries % 4 == 0) {
            putchar('\n');
        }
    }
    putchar('\n');

    // Clean up and exit.
    fclose(input_file);
    fclose(output_file);

    return 0;
}