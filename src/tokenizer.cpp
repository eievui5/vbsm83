#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "exception.hpp"
#include "tokenizer.hpp"

constexpr int MAX_COMMENT = 80;

const char COMMENT[] = "//";
const char BRACKETS[] = "()[]{}";
const char* KEYWORDS[] = {
    "extern", "export", "static",
    "fn", "var",
    "return",
    nullptr
};
// Values which denote the beginning of a number.
const char NUMBERS[] = "0123456789";
const char* OPERATORS[] = {
    "!", "-", "*", "&", "~", "+", "/", "&", "|", "^", "&&", "||", "mod", "<<",
    ">>", "<", ">", "<=", ">=", "!=", "==",
    nullptr
};
const char SINGLES[] = "(){},;";
const char SYMBOLS[] = "!-*&~+-/|^<>=(){}[],;";
const char* TYPES[] = {
    "u8", "u16", "u32", "u64",
    "i8", "i16", "i32", "i64",
    "f32", "f64",
    "p", "farp", "void",
    nullptr
};

int strinstrs(const char* str, const char** strs) {
    for (int i = 0; strs[i] != NULL; i++) {
        if (strcmp(str, strs[i]) == 0) {
            return i;
        }
    }
    return -1;
}

void Token::determine_type() {
    // Determine token type.

    // Check for comments first
    if (string.rfind(COMMENT, 0) != -1) {
        type = TokenType::COMMENT;
        return;
    }

    // Collect single-char tokens.
    if (string.length() == 1) {
        if (string[0] == ',') {
            type = TokenType::COMMA;
            return;
        }
        if (string[0] == ';') {
            type = TokenType::SEMICOLON;
            return;
        }

        if (strchr(BRACKETS, string[0])) {
            type = TokenType::BRACKET;
            return;
        }
    }

    // Keywords
    {
        int keyword = strinstrs(string.c_str(), KEYWORDS);
        if (keyword != -1) {
            type = TokenType::KEYWORD;
            keyword = keyword;
            return;
        }
    }

    // Then operators...
    if (strinstrs(string.c_str(), OPERATORS) >= 0) {
        type = TokenType::OPERATOR;
        return;
    }

    // Types...
    if (strinstrs(string.c_str(), TYPES) >= 0) {
        type = TokenType::TYPE;
        return;
    }

    // Number constants.
    if (strchr(NUMBERS, string[0])) {
        type = TokenType::INT;
        return;
    }

    // Keywords.
    if (strinstrs(string.c_str(), KEYWORDS) >= 0) {
        type = TokenType::KEYWORD;
        return;
    }

    // Fallback onto identifier.
    type = TokenType::IDENTIFIER;;
}

Token* read_token(std::ifstream& input) {
    Token* token {new Token};
    bool alpha_mode;

    while (1) {
        if (input.eof()) {
            break;
        }

        char next_char = input.get();

        if (token->string.length() == 0) {
            // Ignore leading whitespace.
            if (std::isspace(next_char)) {
                continue;
            }

            // Now check for single-char tokens, such as BRACKETS, ",", and ";".
            if (strchr(SINGLES, next_char) != NULL) {
                token->string += next_char;
                break;
            }

            // If the first character is a symbol, then it can be delimited by
            // text (and vice-versa). This means "a+b" is valid, in addition to
            // "a + b".
            alpha_mode = strchr(SYMBOLS, next_char) != NULL;
        }

        // Whitespace delimits all tokens.
        if (std::isspace(next_char)) {
            break;
        }

        if ((strchr(SYMBOLS, next_char) == NULL) == alpha_mode) {
            break;
        }

        // If the token did not end, append to the raw string and continue.
        token->string += next_char;
    }

    token->determine_type();

    // Special handling for comments.
    if (token->type == TokenType::COMMENT) {
        // Copy the rest of the line when a comment appears. This allows the IR
        // to output comments into the assembly code.
        input.seekg(-1, std::ios_base::cur);
        char comment[MAX_COMMENT];
        input.getline(comment, MAX_COMMENT);
        if (!input.fail()) {
            token->string += comment;
        } else {
            delete token;
            return nullptr;
        }
    }

    return token;
}