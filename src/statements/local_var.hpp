#pragma once

#include <memory>
#include <string>

#include "parser.hpp"

class LocalVarDeclaration : public FuncStatement {
  public:
    std::string identifier;
    std::string value;
    VariableType variable_type;

    void compile(std::ostream& outfile, FunctionContext& context);

    ~LocalVarDeclaration() = default;
};