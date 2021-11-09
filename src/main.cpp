#include <getopt.h>
#include <stdio.h>
#include <vector>

#include "exception.hpp"
#include "file.hpp"
#include "parser.hpp"
#include "tokenizer.hpp"

static struct option const longopts[] = {
    {"input",	required_argument,	NULL, 'i'},
    {"output",	required_argument,	NULL, 'o'},
    {NULL,		no_argument,		NULL, 0}
};
static const char shortopts[] = "i:o:";

int main(int argc, char* argv[]) {
    File input_file;
    File output_file;
    char option_char;

    // Parse command-line options.
    while ((option_char = getopt_long_only(argc, argv, shortopts, longopts, NULL)) != -1) {
        switch (option_char) {
            case 'i':
                input_file.open(optarg, "r");
                if (!input_file)
                    error("Error opening file %s!", *optarg);
                break;
            case 'o':
                output_file.open(optarg, "w");
                if (!output_file)
                    error("Error opening file %s!", *optarg);
                break;
        }
    }

    if (!input_file)
        error("Missing input file!");
    if (!output_file)
        error("Missing input file!");

    // Check for errors.
    if (error_count > 0) {
        fprintf(stderr, "CLI failed with %u errors.\n", error_count);
        return 1;
        exit(1);
    }

    TokenList token_list;

    while (!input_file.eof()) {
        Token token = read_token(input_file);
        if (token.type == TokenType::NONE or token.type == TokenType::COMMENT)
            continue;
        token_list.tokens.push_back(token);
    }

    for (auto& i : token_list.tokens)
        puts(i.string.c_str());

    Statement declaration = begin_declaration(token_list);
}