#include <fstream>

#include "exception.hpp"
#include "parser.hpp"

void compile_function(std::ostream& outfile, Function* func) {
    outfile << "; " << *func << '\n';
    switch (func->locality) {
    case DeclLocal::EXPORT:
        outfile << "_" << func->identifier << "::\n";
        break;
    case DeclLocal::STATIC:
        outfile << "__" << func->identifier << ":\n";
        break;
    default:
        break;
    }
}

void compile_variable(std::ostream& outfile, Variable* var) {
    outfile << "; " << *var << '\n';
    warn("Variables are not yet supported.");
    switch (var->locality) {
    case DeclLocal::EXPORT:
        outfile << "_" << var->identifier << "::\n";
        break;
    case DeclLocal::STATIC:
        outfile << "__" << var->identifier << ":\n";
        break;
    default:
        break;
    }
}