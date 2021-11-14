#pragma once

#include "parser.hpp"

class Label : public Statement {
  public:
    std::string identifier;
    StorageClass storage_class;
    std::vector<std::string> trait_list;
    VariableType variable_type;

    ~Label() = default;
};

class Function : public Label {
  public:
    inline Function() { type = StatementType::FUNCTION; }

    void write(std::ostream& os);

    ~Function() = default;
};

class Variable : public Label {
  public:
    inline Variable() { type = StatementType::VARIABLE; }

    void write(std::ostream& os);

    ~Variable() = default;
};