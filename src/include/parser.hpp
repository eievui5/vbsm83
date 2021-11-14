#pragma once

#include <ostream>
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
    virtual void compile(std::ostream& outfile) = 0;

    virtual ~Statement() = default;
};

Statement* begin_declaration(TokenList& token_list);
void parse_token_list(UnitContext& context, TokenList& token_list);
Statement* read_statement(TokenList& token_list);