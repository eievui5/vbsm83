#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "exception.h"
#include "optimizer.h"
#include "parser.h"
#include "statements.h"
#include "varray.h"

static struct option const longopts[] = {
    {"input",  required_argument, NULL, 'i'},
    {"output", required_argument, NULL, 'o'},
    {"ir",     required_argument, NULL, 'r'},
    {NULL}
};
static const char shortopts[] = "i:o:r:";

int main(int argc, char* argv[]) {
    FILE* ir_in = NULL;
    FILE* ir_out = NULL;
    FILE* asm_out = NULL;
    const char* ir_in_path = NULL;
    const char* ir_out_path = NULL;
    const char* asm_out_path = NULL;

    // Parse command-line options.
    for (char option_char; (option_char = getopt_long_only(argc, argv, shortopts, longopts, NULL)) != -1;) {
        switch (option_char) {
        case 'i':
            ir_in_path = optarg;
            break;
        case 'o':
            asm_out_path = optarg;
            break;
        case 'r':
            ir_out_path = optarg;
            break;
        }
    }

    // Verify inputs.
    // An input IR file is required.
    if (ir_in_path == NULL) {
        error("Missing input file path.");
    } else {
    	ir_in = fopen(ir_in_path, "r");
    	if (ir_in == NULL)
    		error("Failed to open %s.", ir_in_path);
    }
    // Output optimized IR file is optional.
	if (ir_out_path != NULL) {
        if (strcmp(ir_out_path, "-") == 0)
            ir_out = stdout;
        else
    	    ir_out = fopen(ir_out_path, "w");
    	if (ir_out == NULL)
    		error("Failed to open %s.", ir_out_path);
    }
    // Assembly output is also optional, in case only optimized IR is needed.
	if (asm_out_path != NULL) {
        if (strcmp(asm_out_path, "-") == 0)
            asm_out = stdout;
        else
    	    asm_out = fopen(asm_out_path, "w");
    	if (asm_out == NULL)
    		error("Failed to open %s.", asm_out_path);
    }
    if (asm_out == NULL && ir_out == NULL)
        warn("No output files were provided. Performing a dry run.");

    // Check for CLI errors before processing.
    errcheck();

    // Parse the input IR file.
    Declaration** declaration_list = fparse_textual_ir(ir_in);

    for (int i = 0; i < va_len(declaration_list); i++)
        if (declaration_list[i]->is_fn)
            remove_unused_blocks((Function*) declaration_list[i]);

    // Check for errors before writing to output files.
    errcheck();

    // If an IR output file was provided, print to it now.
    if (ir_out != NULL)
        for (int i = 0; i < va_len(declaration_list); i++)
            fprint_declaration(ir_out, declaration_list[i]);

    // Final clean up before exit.
    for (int i = 0; i < va_len(declaration_list); i++)
        free_declaration(declaration_list[i]);
    va_free(declaration_list);

    fclose(ir_in);
    if (ir_out != NULL)
        fclose(ir_out);
    if (asm_out != NULL)
        fclose(asm_out);
}
