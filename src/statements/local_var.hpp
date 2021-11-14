#pragma once

#include <string>

#include "parser.hpp"

class LocalVarDeclaration : public Statement {
  public:
    std::string identifier;
    VariableType variable_type;

    void compile(std::ostream& outfile);

    ~LocalVarDeclaration() = default;
};

class LocalVarAssignment : public Statement {
  public:
    std::string identifier;
    std::string value;

    void compile(std::ostream& outfile);
};