#pragma once

#include "tokenizer.hpp"

enum class StatementType {NONE, FUNCTION, RETURN};

class Statement {
public:
    std::string string;
    StatementType type = StatementType::NONE;
};

class Function : public Statement {
public:
    std::string identifier;
    DeclLocal locality;
    VariableType return_type;
    std::vector<std::string> trait_list;

    Function() {type = StatementType::FUNCTION;}
};

class Return : public Statement {
public:
    bool is_const;
    union {
        uint64_t constant_value;
        std::string identifier;
    };
    VariableType return_type;
};

std::ostream& operator<<(std::ostream& os, Function function);
Statement begin_declaration(TokenList& token_list);