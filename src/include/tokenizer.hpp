#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "file.hpp"

enum class Keyword {
    FN,
    VAR,
    RETURN,
};
enum class DeclLocal { EXTERN, EXPORT, STATIC };

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
    LOCALITY,
};

class Token {
  public:
    TokenType type = TokenType::NONE;
    // I want to factor this union into functions rather than pre-processing it.
    union {
        Keyword keyword;
        DeclLocal locality;
    };
    std::string string;

    void determine_type();
};

class TokenList {
  public:
    std::vector<Token> tokens;
    size_t index = 0;

    inline void expect(std::string str) {
        if (peek_token().string != str)
            fatal("Expected %s, got %s.", str.c_str(), peek_token().string.c_str());
        index++;
    }

    inline void expect(std::string str, std::string message) {
        if (peek_token().string != str)
            fatal(message.c_str());
        index++;
    }

    inline Token& get_token() { return tokens.at(index++); }
    inline Token& peek_token() { return tokens.at(index); }
    inline int remaining() { return tokens.size() - index; }
};

extern const char COMMENT[];
extern const char BRACKETS[];
extern const char* LOCALITY[];
extern const char* KEYWORDS[];
extern const char NUMBERS[];
extern const char* OPERATORS[];
extern const char SINGLES[];
extern const char SYMBOLS[];
extern const char WHITESPACE[];

Token read_token(std::ifstream& infile);
int strinstrs(std::string& str, const char** strs);
