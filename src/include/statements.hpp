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
        // How the variable is stored.
        StorageClass storage_class;
        // Frontend-supplied strings; can be ignored if not recognized.
        std::vector<std::string> traits;
        VariableType type;
        std::string identifier;
    };

    class Function : public Variable {
    public:
        // How the function is stored.
        StorageClass storage_class;
        // Frontend-supplied strings; can be ignored if not recognized.
        std::vector<std::string> traits;
        VariableType type;
        std::string identifier;
    };

    class Assignment : public Statement {
    public:
        std::string dest;
        // `source` is either a variable or a constant.
        std::string source;
    };

    class BinOp : public Statement {
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

    class Return : public Statement {
    public:
        // True if `source` is constant.
        bool is_const;
        std::string source;
    };
};

typedef std::vector<stmnt::Statement> StatementList;

std::vector<std::vector<Token*>> collect_statements(TokenList& token_list);
StatementList parse_statements(TokenList& token_list);
