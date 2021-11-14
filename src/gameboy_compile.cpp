#include <ostream>

#include "statements/label.hpp"
#include "statements/local_var.hpp"
#include "statements/return.hpp"

void Label::compile(std::ostream& outfile) {
    outfile << '_';
    if (storage_class == StorageClass::STATIC)
        outfile << '_';
    outfile << identifier << ':';
    if (storage_class == StorageClass::EXPORT)
        outfile << ':';
    outfile << '\n';
}

void LocalVarDeclaration::compile(std::ostream& outfile) {
    outfile << '_';
}

void LocalVarAssignment::compile(std::ostream& outfile) {
    outfile << '_';
}

void ReturnStatement::compile(std::ostream& outfile) {
    outfile << '_';
}