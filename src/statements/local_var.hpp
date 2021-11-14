#pragma once

#include <memory>
#include <string>

#include "parser.hpp"

class LocalVarDeclaration : public FuncStatement {
  public:
    std::string identifier;
    VariableType variable_type;

    void compile(std::ostream& outfile, FunctionContext& context);

    ~LocalVarDeclaration() = default;
};

class LocalVarAssignment : public FuncStatement {
  public:
    std::string identifier;
    std::string value;

    void compile(std::ostream& outfile, FunctionContext& context);
};