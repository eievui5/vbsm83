#pragma once

#include <string>
#include <vector>

#include "file.hpp"

enum class Keyword {
    EXTERN, EXPORT, STATIC,
    FN, VAR,
    RETURN,
};

enum class TokenType {
    NONE,
    COMMENT,
    OPERATOR,
    TYPE,
    IDENTIFIER,
    TRAIT,
    BRACKET,
    COMMA,
    SEMICOLON,
    INT,
    FLOAT,
    KEYWORD,
};

enum class VariableType {
    U8, U16, U32, U64,
    I8, I16, I32, I64,
    F32, F64, PTR, FARPTR,
    VOID,
};

class Token {
public:
    TokenType type = TokenType::NONE;
    Keyword keyword;
    std::string string;

    void determine_type();
};

class TokenList {
public:
    std::vector<Token> tokens;
    size_t index = 0;

    inline Token& get_token() {
        return tokens.at(index++);
    }

    inline Token& peek_token() {
        return tokens.at(index);
    }
};

extern const char COMMENT[];
extern const char BRACKETS[];
extern const char* KEYWORDS[];
extern const char NUMBERS[];
extern const char* OPERATORS[];
extern const char SINGLES[];
extern const char SYMBOLS[];
extern const char* TYPES[];
extern const char WHITESPACE[];

Token read_token(File& infile);
int strinstrs(const char* str, const char** strs);