#include <getopt.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "exception.hpp"
#include "gbcomp.hpp"
#include "parser.hpp"
#include "tokenizer.hpp"

static struct option const longopts[] = {
    {"input",	required_argument,	NULL, 'i'},
    {"output",	required_argument,	NULL, 'o'},
    {NULL,		no_argument,		NULL, 0}
};
static const char shortopts[] = "i:o:";

int main(int argc, char* argv[]) {
    std::ifstream input_file;
    std::ofstream output_file;
    char option_char;

    // Parse command-line options.
    while ((option_char = getopt_long_only(argc, argv, shortopts, longopts, NULL)) != -1) {
        switch (option_char) {
        case 'i':
            input_file.open(optarg);
            if (!input_file.is_open())
                error("Error opening file %s!", optarg);
            break;
        case 'o':
            output_file.open(optarg);
            if (!output_file.is_open())
                error("Error opening file %s!", optarg);
            break;
        }
    }

    if (!input_file.is_open())
        error("Missing input file!");
    if (!output_file.is_open())
        error("Missing input file!");

    // Check for errors.
    if (error_count > 0)
        fatal("CLI failed with %u errors.\n", error_count);

    TokenList token_list;

    while (!input_file.eof()) {
        Token token = read_token(input_file);
        if (token.type == TokenType::NONE or token.type == TokenType::COMMENT)
            continue;
        token_list.tokens.push_back(token);
    }

    for (auto& i : token_list.tokens)
        std::cout << i.string << ", ";
    std::cout << '\n';

    std::unique_ptr<Statement> declaration {begin_declaration(token_list)};
    switch (declaration->type) {
    case StatementType::FUNCTION:
        compile_function(output_file, (Function*) declaration.get());
        break;
    case StatementType::VARIABLE:
        compile_variable(output_file, (Variable*) declaration.get());
        break;
    default:
        break;
    }
}