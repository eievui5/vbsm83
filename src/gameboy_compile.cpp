#include <ostream>

#include "statements/label.hpp"
#include "statements/local_var.hpp"
#include "statements/return.hpp"
#include "types.hpp"

void Label::compile(std::ostream& outfile) {
    // Output an info comment.
    outfile << "; " << STORAGE_CLASS[(int) storage_class] << ' ' << identifier << " [[ ";
    for (auto& i : trait_list) {
        outfile << i << ", ";
    }
    outfile << "]]\n";

    // Output label if needed.
    if (storage_class != StorageClass::EXTERN) {
        outfile << '_';
        if (storage_class == StorageClass::STATIC)
            outfile << '_';
        outfile << identifier << ':';
        if (storage_class == StorageClass::EXPORT)
            outfile << ':';
        outfile << '\n';
    }
}

void Variable::compile(std::ostream& outfile) {
    Label::compile(outfile);
    outfile << "\tDS " << get_type(variable_type).size << '\n';
}

void Function::compile(std::ostream& outfile) {
    Label::compile(outfile);
    for (auto* i : unit_block.statements) {
        i->compile(outfile);
    }
}

void LocalVarDeclaration::compile(std::ostream& outfile) {
    outfile << "\t; " << get_type(variable_type).str << ' ' << identifier << '\n';
}

void LocalVarAssignment::compile(std::ostream& outfile) {
    outfile << "\t; " << identifier << " = " << value << '\n';
}

void ReturnStatement::compile(std::ostream& outfile) {
    outfile << "\t; return " << value << '\n';
    outfile << "\tret\n";
}