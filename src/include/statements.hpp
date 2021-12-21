#pragma once

#include <string>
#include <vector>

#include "tokenizer.hpp"
#include "types.hpp"

class Statement {
public:
    std::vector<Token>* tokens;

    void print(std::ostream& outfile) {
        for (auto& i : *tokens)
            outfile << i.string << ' ';
    }
};

typedef std::vector<Statement> StatementBlock;

class AssignStatement : public Statement {
public:
    std::string dest;
    // `source` is either a variable or a constant.
    std::string source;
};

class BinOpStatement : public Statement {
public:
    // The operation being performed.
    BinOpType type;
    // True if the `rhs` is constant.
    bool is_const;
    VariableType dest_type;
    std::string dest;
    // `lhs` is always a variable.
    std::string lhs;
    // `rhs` is either a variable or a constant.
    std::string rhs;
};

class ReturnStatement : public Statement {
public:
    // True if `source` is constant.
    bool is_const;
    std::string source;
};

class VariableDeclaration : public Statement {
public:
    // How the variable is stored.
    StorageClass storage_class;
    // Frontend-supplied strings; can be ignored if not recognized.
    std::vector<std::string> traits;
    VariableType type;
    std::string identifier;
};

// Functions act much like constant variables but contain statements which
// determine their value.
class FunctionDeclaration : public VariableDeclaration {
public:
    StatementBlock block;
};

std::vector<std::vector<Token*>> collect_statements(TokenList& token_list);
StatementBlock parse_statements(TokenList& token_list);
