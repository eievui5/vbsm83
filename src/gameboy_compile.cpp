#include <ostream>

#include "statements/label.hpp"
#include "statements/local_var.hpp"
#include "statements/return.hpp"
#include "tokenizer.hpp"
#include "types.hpp"

void LocalVariable::assign(std::ostream& outfile, FunctionContext& context, std::string identifier, std::string value) {
    outfile << "\tld " << context.local_vars[identifier]->container->name << ", ";
    if (determine_token_type(value) == TokenType::INT) {
        outfile << value;
    } else {
        outfile << context.local_vars[value]->container->name;
    }
    outfile << '\n';
}

void Label::define(std::ostream& outfile) {
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

void Variable::define(std::ostream& outfile) {
    Label::define(outfile);
    outfile << "\tDS " << get_type(variable_type).size << '\n';
}

void Function::define(std::ostream& outfile) {
    Label::define(outfile);
    for (auto* i : unit_block.statements) {
        FuncStatement* func = dynamic_cast<FuncStatement*>(i);
        if (!func)
            error("Non-function statement found in function block.");
        func->compile(outfile, unit_block);
    }
}

void LocalVarDeclaration::compile(std::ostream& outfile, FunctionContext& context) {
    LocalVariable* local_var = new LocalVariable(context.local_vars, get_type(variable_type).size);
    context.local_vars.reserve(1);
    context.local_vars.emplace(identifier, local_var);
    outfile << "\t; " << get_type(variable_type).str << ' ' << identifier << '\n';
}

void LocalVarAssignment::compile(std::ostream& outfile, FunctionContext& context) {
    outfile << "\t; " << identifier << " = " << value << '\n';
    context.local_vars[identifier]->assign(outfile, context, identifier, value);
}

void ReturnStatement::compile(std::ostream& outfile, FunctionContext& context) {
    outfile << "\t; return " << value << '\n';
    outfile << "\tld ";

    const BackendType& return_type = get_type(context.return_type);
    switch (return_type.size) {
    case 1:
        outfile << 'c';
        break;
    case 2:
        outfile << "bc";
        break;
    default:
        fatal("Unhandled return type size: %i", return_type.size);
    }

    outfile << ", ";

    if (determine_token_type(value) == TokenType::INT) {
        outfile << value;
    } else {
        outfile << context.local_vars[value]->container->name;
    }
    outfile << "\n\tret\n";
}