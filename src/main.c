#include <getopt.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
#include "exception.h"
#include "parser.h"

static struct option const longopts[] = {
    {"info",    no_argument,        NULL, 'I'},
    {"input",	required_argument,	NULL, 'i'},
    {"output",	required_argument,	NULL, 'o'},
    {NULL,		no_argument,		NULL, 0}
};
static const char shortopts[] = "Ii:o:";

int main(int argc, char* argv[]) {
    FILE* input_file = NULL;
    FILE* output_file = NULL;
    char option_char;

    // Parse command-line options.
    while ((option_char = getopt_long_only(argc, argv, shortopts, longopts, NULL)) != -1) {
        switch (option_char) {
            case 'I':
                enable_info = true;
                break;
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
        fprintf(stderr, "CLI failed with %u errors.\n", error_count);
        return 1;
    }

    Context* context = get_context(input_file);

    // Check for errors.
    if (error_count > 0) {
        fprintf(stderr, "Tokenization failed with %u errors.\n", error_count);
        return 1;
    }

    compile_context(context, output_file);

    // Check for errors.
    if (error_count > 0) {
        fprintf(stderr, "Compilation failed with %u errors.\n", error_count);
        return 1;
    }

    free_context(context);

    // Clean up and exit.
    fclose(input_file);
    fclose(output_file);

    return 0;
}