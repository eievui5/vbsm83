#pragma once

#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include "exception.hpp"

class TokenList;

enum class StorageClass { EXTERN, EXPORT, STATIC };

enum class TokenType {
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

TokenType determine_token_type(const std::string& string);

class Token {
public:
    TokenType type;
    std::string string;

    const char* c_str() { return string.c_str(); }
    void determine_type() { type = determine_token_type(string); }
    int64_t read_int() { return strtoll(c_str(), NULL, 0); }
    uint64_t read_uint() { return strtoull(c_str(), NULL, 0); }
};

typedef std::vector<Token> TokenCluster;

class TokenList {
public:
    // A list of tokens, seperated into individual statements
    std::vector<std::vector<Token>> clusters;
    size_t index = 0;

    void print(std::ostream& outfile) {
        for (auto& i : clusters) {
            for (auto& j : i)
                outfile << j.string << ", ";
            outfile << '\n';
        }
    }

    void seek(int _index) { index = _index; }
    // Returns a cluster of tokens.
    TokenCluster& get_cluster() { return clusters[index++]; }
    int remaining() { return clusters.size() - index; }
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

bool read_token(std::istream& infile, Token& token);
int strinstrs(const std::string& str, const char** strs);
