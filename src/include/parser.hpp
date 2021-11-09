#pragma once

#include "tokenizer.hpp"

class Statement {
public:
    std::string string;
};

class Function : public Statement {
public:
    VariableType return_type;
    std::vector<std::string> trait_list;
};

Statement begin_declaration(TokenList& token_list);