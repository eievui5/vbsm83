#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "exception.hpp"

class TokenList;

enum class Keyword {
    FN,
    VAR,
    RETURN,
};
enum class StorageClass { EXTERN, EXPORT, STATIC };

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
    STORAGE_CLASS,
    CONTROL,
};

class Token {
  public:
    TokenType type = TokenType::NONE;
    // The union is deprecated, please avoid using.
    union {
        Keyword keyword;
        StorageClass storage_class;
    };
    std::string string;

    void determine_type();

    inline const char* c_str() {return string.c_str();}
};

/* Control class for handling optimizations done by analysis.
Provides a compile function which handles output based on the current context,
allowing simple, encapsulated handling of special tokens.
*/
class ControlToken : public Token {
  public:

    ControlToken() {type = TokenType::CONTROL;}

    virtual void compile(std::ostream outfile, TokenList& token_list);
};

class TokenList {
  public:
    std::vector<std::reference_wrapper<Token>> tokens;
    size_t index = 0;

    inline Token& expect(std::string str) {
        if (peek_token().string != str)
            fatal("Expected %s, got %s.", str.c_str(), peek_token().c_str());
        return get_token();
    }

    inline Token& expect(std::string str, const char* message) {
        if (peek_token().string != str)
            fatal(message);
        return get_token();
    }

    inline Token& expect_type(TokenType type) {
        if (peek_token().type != type)
            fatal("Unexpected token %s.");
        return get_token();
    }

    inline Token& expect_type(TokenType type, const char* message) {
        if (peek_token().type != type)
            fatal(message);
        return get_token();
    }

    inline Token& get_token() { return tokens.at(index++); }
    inline Token& peek_token() { return tokens.at(index); }
    inline int remaining() { return tokens.size() - index; }
};

extern const char COMMENT[];
extern const char BRACKETS[];
extern const char* STORAGE_CLASS[];
extern const char* KEYWORDS[];
extern const char NUMBERS[];
extern const char* OPERATORS[];
extern const char SINGLES[];
extern const char SYMBOLS[];
extern const char WHITESPACE[];

Token& read_token(std::ifstream& infile);
int strinstrs(std::string& str, const char** strs);
