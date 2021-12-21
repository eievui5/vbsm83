#include <fstream>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <vector>

#include "exception.hpp"
#include "preprocessor.hpp"
#include "register_allocation.hpp"
#include "statements.hpp"
#include "tokenizer.hpp"

using std::cout;

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

    // Process input.
    std::stringstream processed_infile = preprocess(input_file);
    input_file.close();
    TokenList token_list;
    token_list.clusters.push_back({});

    while (!processed_infile.eof()) {
        Token token;
        if (read_token(processed_infile, token))
            break;
        warn("%s", token.c_str());
        char first_char = token.string[0];
        if (first_char == ';') {
            token_list.clusters.push_back({});
            continue;
        }
        // Copy the token into the cluster.
        token_list.clusters.back().push_back(token);
        if (first_char == '}' || first_char == '{')
            token_list.clusters.push_back({});
    }

    // Output debug info.
    cout << "Pre-processed infile:\n";
    cout << processed_infile.str();
    cout << "Token list:\n";
    token_list.print(cout);
    cout << "\n\n";
    StatementBlock statements = parse_statements(token_list);
    cout << "Recognized statements:\n";
    for (auto& i : statements) {
        i.print(cout);
        cout << '\n';
    }
}
