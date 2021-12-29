#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "exception.h"
#include "parser.h"
#include "statements.h"
#include "varray.h"

static struct option const longopts[] = {
    {"input",  required_argument, NULL, 'i'},
    {"output", required_argument, NULL, 'o'},
    {NULL}
};
static const char shortopts[] = "i:o:v";

int main(int argc, char* argv[]) {
    FILE* infile = NULL;
    FILE* outfile = NULL;
    const char* infile_path = NULL;
    const char* outfile_path = NULL;
    char option_char;

    // Parse command-line options.
    while ((option_char = getopt_long_only(argc, argv, shortopts, longopts, NULL)) != -1) {
        switch (option_char) {
        case 'i':
            infile_path = optarg;
            break;
        case 'o':
            outfile_path = optarg;
            break;
        }
    }

    if (infile_path == NULL) {
        error("Missing input file path.");
    } else {
    	infile = fopen(infile_path, "r");
    	if (infile == NULL)
    		error("Failed to open %s.", infile_path);
    }
	if (outfile_path == NULL) {
        error("Missing output file path.");
    } else {
        if (strcmp(outfile_path, "-") == 0)
            outfile = stdout;
        else
    	    outfile = fopen(outfile_path, "w");
    	if (outfile == NULL)
    		error("Failed to open %s.", outfile_path);
    }

    errcheck();

    Declaration** declaration_list = fparse_textual_ir(infile);
    for (int i = 0; i < va_len(declaration_list); i++) {
        fprint_declaration(outfile, declaration_list[i]);
        free_declaration(declaration_list[i]);
    }
    va_free(declaration_list);

    fclose(infile);
    fclose(outfile);
}
