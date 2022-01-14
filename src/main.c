#define _POSIX_C_SOURCE 200809L
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "exception.h"
#include "optimizer.h"
#include "parser.h"
#include "registers.h"
#include "statements.h"
#include "varray.h"

static struct option const longopts[] = {
    {"ansi",     no_argument,       NULL, 'a'},
    {"optimize", required_argument, NULL, 'f'},
    {"help",     no_argument,       NULL, 'h'},
    {"input",    required_argument, NULL, 'i'},
    {"output",   required_argument, NULL, 'o'},
    {"ir",       required_argument, NULL, 'r'},
    {NULL}
};
static const char shortopts[] = "af:hi:o:r:";

void print_help(char* name) {
    printf("usage:\n  %s -i <infile> -o <outfile>\n", name);
    puts("options:\n"
         "  -a --ansi     Toggle ANSI terminal support.\n"
         "  -f --optimize Enable or disable certain optimizations. Enter -fhelp for help.\n"
         "  -h --help     Show this message.\n"
         "  -i --input    Path to the input IR file.\n"
         "  -o --output   Path to the output assembly file.\n"
         "  -r --ir       Path to the output optimized IR file.");
}

int main(int argc, char* argv[]) {
    FILE* ir_in = NULL;
    FILE* ir_out = NULL;
    FILE* asm_out = NULL;
    const char* ir_in_path = NULL;
    const char* ir_out_path = NULL;
    const char* asm_out_path = NULL;

    // Check if stderr is a tty.
    ansi_exceptions = isatty(fileno(stderr));

    // If the program is run without arguments, assume the user is asking for
    // help.
    if (argc < 2) {
        warn("No arguments provided. Printing help.");
        puts("DCC SM83 backend");
        print_help(argv[0]);
        exit(0);
    }

    // Parse command-line options.
    char option_char;
    while ((option_char = getopt_long_only(argc, argv, shortopts, longopts, NULL)) != -1) {
        switch (option_char) {
        case 'a':
            ansi_exceptions ^= true;
            break;
        case 'f':
            if (strequ(optarg, "help")) {
                print_opt_help();
                // If fhelp is the only argument, simply end execution.
                if (argc == 2)
                    exit(0);
            } else {
                parse_opt_flag(optarg);
            }
            break;
        case 'h':
            // When explicitly asked for help, output additional info.
            puts("DCC SM83 backend");
            print_help(argv[0]);
            // If help is the only argument, simply end execution.
            if (argc == 2)
                exit(0);
            break;
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

    // An input IR file is required.
    if (ir_in_path == NULL) {
        error("Missing input file path.");
    } else {
    	ir_in = fopen(ir_in_path, "r");
    	if (ir_in == NULL)
    		error("Failed to open %s.", ir_in_path);
    }

    // Output optimized IR file is optional.
	if (ir_out_path) {
        if (strequ(ir_out_path, "-"))
            ir_out = stdout;
        else
    	    ir_out = fopen(ir_out_path, "w");
    	if (ir_out == NULL)
    		error("Failed to open %s.", ir_out_path);
    }

    // Assembly output is also optional, in case only optimized IR is needed.
	if (asm_out_path) {
        if (strequ(asm_out_path, "-"))
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
    optimize_ir(declaration_list);
    for (size_t i = 0; i < va_len(declaration_list); i++) {
        if (declaration_list[i]->is_fn) {
            analyze_var_usage((Function*) declaration_list[i]);
            fprint_var_usage(stdout, (Function*) declaration_list[i]);
        }
    }

    // Check for errors before writing to output files.
    errcheck();

    // If an IR output file was provided, print to it now.
    if (ir_out) {
        if (va_len(declaration_list) > 0)
            fprint_declaration(ir_out, declaration_list[0]);
        for (size_t i = 1; i < va_len(declaration_list); i++) {
            fputc('\n', ir_out);
            fprint_declaration(ir_out, declaration_list[i]);
        }
    }

    // Final clean up before exit.
    for (size_t i = 0; i < va_len(declaration_list); i++)
        free_declaration(declaration_list[i]);
    va_free(declaration_list);

    fclose(ir_in);
    if (ir_out)
        fclose(ir_out);
    if (asm_out)
        fclose(asm_out);
}
