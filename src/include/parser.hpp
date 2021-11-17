#pragma once

#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "exception.hpp"
#include "register_allocation.hpp"
#include "tokenizer.hpp"
#include "types.hpp"

class FunctionContext;

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

class LocalVar : public FuncStatement {
  public:
    const CPUReg* container;
    std::string identifier;
    std::string value;
    VariableType variable_type;
    int size;
    // How many times is the variable referenced after its assignment?
    int use_count;
    bool is_returned;

    void compile(std::ostream& outfile, FunctionContext& context);
    void print_info(std::ostream& out);
};

class UnitContext {
  public:
    std::vector<Statement*> statements;

    inline void append(Statement* statement) { statements.push_back(statement); }

    ~UnitContext();
};

class FunctionContext : public UnitContext {
  public:
    std::unordered_map<std::string, LocalVar*> local_vars;
    VariableType return_type;
};

Statement* begin_declaration(TokenList& token_list);
void parse_token_list(UnitContext& context, TokenList& token_list);
Statement* read_statement(TokenList& token_list);
