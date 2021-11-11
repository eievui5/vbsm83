#pragma once

#include "tokenizer.hpp"
#include "types.hpp"

enum class StatementType { NONE, FUNCTION, VARIABLE, RETURN };

class Statement {
  public:
    StatementType type = StatementType::NONE;
};

class Declaration : public Statement {
  public:
    std::string identifier;
    StorageClass storage_class;
    std::vector<std::string> trait_list;
};

class Function : public Declaration {
  public:
    VariableType return_type;

    inline Function() { type = StatementType::FUNCTION; }
};

class Variable : public Declaration {
  public:
    VariableType variable_type;

    inline Variable() { type = StatementType::VARIABLE; }
};

class Return : public Statement {
  public:
    // I'd like to factor out this struct and just figure things out based on
    // what comes after the return.
    bool is_const;
    union {
        uint64_t constant_value;
        std::string identifier;
    };
    VariableType return_type;
};

std::ostream& operator<<(std::ostream& os, Function function);
std::ostream& operator<<(std::ostream& os, Variable variable);
Statement* begin_declaration(TokenList& token_list);