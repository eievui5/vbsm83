#pragma once

#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "exception.hpp"
#include "register_allocation.hpp"
#include "tokenizer.hpp"
#include "types.hpp"

class Statement;

class UnitContext {
  public:
    std::vector<Statement*> statements;

    inline void append(Statement* statement) { statements.push_back(statement); }

    ~UnitContext();
};

class FunctionContext : public UnitContext {
  public:
    std::unordered_map<std::string, LocalVariable*> local_vars;
    VariableType return_type;
};

// Statement types are deprecated, please do not use.
enum class StatementType { NONE, FUNCTION, VARIABLE, RETURN };

class Statement {
  public:
    virtual void define(std::ostream& outfile) = 0;

    virtual ~Statement() = default;
};

class FuncStatement : public Statement {
  public:
    virtual void compile(std::ostream& outfile, FunctionContext& context) = 0;

    void define(std::ostream& outfile) { fatal("Function statments may only be compiled, not defined."); }
};

Statement* begin_declaration(TokenList& token_list);
void parse_token_list(UnitContext& context, TokenList& token_list);
Statement* read_statement(TokenList& token_list);