#include <fstream>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <vector>

#include "analysis.hpp"
#include "compile.hpp"
#include "exception.hpp"
#include "parser.hpp"
#include "register_allocation.hpp"
#include "tokenizer.hpp"

static struct option const longopts[] = {
    {  "input", required_argument, NULL, 'i'},
    { "output", required_argument, NULL, 'o'},
    {"verbose", required_argument, NULL, 'v'},
    {     NULL,       no_argument, NULL,   0}
};
static const char shortopts[] = "i:o:v";

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
        case 'v':
            enable_info = true;
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
        Token* token = read_token(input_file);
        if (token->type == TokenType::NONE or token->type == TokenType::COMMENT) {
            delete token;
            continue;
        }
        token_list.tokens.push_back(token);
    }

    if (info()) {
        for (Token* i : token_list.tokens)
            std::cout << i->string << ", ";
        std::cout << '\n';
    }

    UnitContext root_context;
    parse_token_list(root_context, token_list);
    analyze_unit(root_context);
    //compile(root_context, output_file);
}
