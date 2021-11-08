#include <cstdio>
#include <fstream>
#include <iostream>
#include <getopt.h>

#include "exception.hpp"
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
                input_file.open(optarg, std::ifstream::in);
                if (!input_file.is_open()) {
                    error("Error opening file %s!", *optarg);
                }
                break;
            case 'o':
                output_file.open(optarg, std::ofstream::out);
                if (!output_file.is_open()) {
                    error("Error opening file %s!", *optarg);
                }
                break;
        }
    }

    if (!input_file.is_open()) {
        error("Missing input file!");
    }

    if (!output_file.is_open()) {
        error("Missing input file!");
    }

    // Check for errors.
    if (error_count > 0) {
        std::fprintf(stderr, "CLI failed with %u errors.\n", error_count);
        return 1;
    }

    while (1) {
        std::unique_ptr<Token> token {read_token(input_file)};
        if (token == nullptr)
            break;
        std::cout << token->string << '\n';
    }

    return 0;
}