#pragma once

#include <string>
#include <vector>

#include "tokenizer.hpp"
#include "types.hpp"

namespace stmnt {
    class Statement {
    public:
        std::vector<Token*> tokens;

        void print(std::ostream& outfile) {
            for (auto& i : tokens)
                outfile << i->string << ' ';
        }
    };

    class Variable : public Statement {
    public:
        StorageClass storage_class;
        std::vector<std::string> traits;
        VariableType type;
        std::string identifier;
    };

    class Function : public Variable {
    public:
        StorageClass storage_class;
        std::vector<std::string> traits;
        VariableType type;
        std::string identifier;
    };

    class Assignment : public Statement {
    public:
        std::string dest;
        std::string source;
    };

    class BinOp : public Statement {
    public:
        BinOpType type; // Which operation is being performed?
        bool is_const; // Is the rhs constant?
        VariableType dest_type;
        std::string dest; // The name of the variable for storing the result.
        std::string lhs; // This is always a variable.
        std::string rhs; // Either a constant or a variable
    };

    class Return : public Statement {
    public:
        bool is_const;
        std::string source;
    };
};

class StatementList {
    std::vector<stmnt::Statement*> statements;
};

std::vector<std::vector<Token*>> collect_statements(TokenList& token_list);
std::vector<stmnt::Statement> parse_statements(TokenList& token_list);
