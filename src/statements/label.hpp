#pragma once

#include "parser.hpp"

class Label : public Statement {
  public:
    std::string identifier;
    StorageClass storage_class;
    std::vector<std::string> trait_list;
};

class Function : public Label {
  public:
    VariableType return_type;

    inline Function() { type = StatementType::FUNCTION; }
};

class Variable : public Label {
  public:
    VariableType variable_type;

    inline Variable() { type = StatementType::VARIABLE; }
};

std::ostream& operator<<(std::ostream& os, Function& function);
std::ostream& operator<<(std::ostream& os, Variable& variable);