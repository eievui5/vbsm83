#pragma once

#include "parser.hpp"

class Label : public Statement {
  public:
    std::string identifier;
    StorageClass storage_class;
    std::vector<std::string> trait_list;
    VariableType variable_type;

    void compile(std::ostream& outfile);

    ~Label() = default;
};

class Function : public Label {
  public:
    UnitContext unit_block;

    void compile(std::ostream& outfile);

    ~Function() = default;
};

class Variable : public Label {
  public:
    void compile(std::ostream& outfile);

    ~Variable() = default;
};