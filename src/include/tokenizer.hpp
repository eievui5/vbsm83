#pragma once

#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include "exception.hpp"

class TokenList;

enum class StorageClass { EXTERN, EXPORT, STATIC };

enum class TokenType {
    NONE,
    BINARY_OPERATOR,
    UNARY_OPERATOR,
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

enum class BinOpType {
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    B_AND,
    B_OR,
    B_XOR,
    L_AND,
    L_OR,
    LSH,
    RSH,
    LESS,
    GREATER,
    LESS_EQU,
    GREATER_EQU,
    NOT_EQU,
    EQU,
};

TokenType determine_token_type(std::string string);

class Token {
  public:
    TokenType type = TokenType::NONE;
    std::string string;

    const char* c_str() { return string.c_str(); }
    void determine_type() { type = determine_token_type(string); }
    int64_t read_int() { return strtoll(c_str(), NULL, 0); }
    uint64_t read_uint() { return strtoll(c_str(), NULL, 0); }
};

/* Control class for handling optimizations done by analysis.
Provides a compile function which handles output based on the current context,
allowing simple, encapsulated handling of special tokens.
*/
class ControlToken : public Token {
  public:
    ControlToken() { type = TokenType::CONTROL; }

    virtual void compile(std::ostream& outfile, TokenList& token_list);
};

class TokenList {
  public:
    std::vector<Token*> tokens;
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

    void print(std::ostream& outfile) {
        for (Token* i : tokens) {
            outfile << i->c_str() << ", ";
        }
    }

    inline void seek(int _index) { index = _index; }
    inline Token& get_token() { return *tokens.at(index++); }
    inline Token& peek_token() { return *tokens.at(index); }
    inline Token& peek_token(int lookahead) { return *tokens.at(index + lookahead); }
    inline int remaining() { return tokens.size() - index; }

    inline ~TokenList() {
        for (Token* i : tokens)
            delete i;
    }
};

extern const char BRACKETS[];
extern const char* STORAGE_CLASS[];
extern const char* KEYWORDS[];
extern const char NUMBERS[];
extern const char* BIN_OPS[];
extern const char* UN_OPS[];
extern const char SINGLES[];
extern const char SYMBOLS[];
extern const char WHITESPACE[];

Token* read_token(std::istream& infile);
int strinstrs(std::string& str, const char** strs);
