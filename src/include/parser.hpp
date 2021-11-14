#pragma once

#include <vector>

#include "tokenizer.hpp"
#include "types.hpp"

class Statement;

class UnitContext {
  public:
    std::vector<Statement*> statements;

    inline void append(Statement* statement) { statements.push_back(statement); }

    ~UnitContext();
};

// Statement types are deprecated, please do not use.
enum class StatementType { NONE, FUNCTION, VARIABLE, RETURN };

class Statement {
  public:
    StatementType type = StatementType::NONE;

    virtual ~Statement() = default;
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

    ~Return() = default;
};

Statement* begin_declaration(TokenList& token_list);
void parse_token_list(UnitContext& context, TokenList& token_list);
Statement* read_statement(TokenList& token_list);