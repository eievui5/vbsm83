#pragma once

#include "parser.hpp"

class Label : public Statement {
  public:
    std::string identifier;
    StorageClass storage_class;
    std::vector<std::string> trait_list;
    VariableType variable_type;

    void define(std::ostream& outfile);

    ~Label() = default;
};

class Function : public Label {
  public:
    FunctionContext unit_block;

    void define(std::ostream& outfile);

    ~Function() = default;
};

class Variable : public Label {
  public:
    void define(std::ostream& outfile);

    ~Variable() = default;
};