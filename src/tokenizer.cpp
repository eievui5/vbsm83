#include <ctype.h>
#include <iostream>
#include <string>
#include <string.h>

#include "exception.hpp"
#include "tokenizer.hpp"
#include "types.hpp"

const char BRACKETS[] = "()[]{}";
const char* KEYWORDS[] = {"fn", "var", "return", nullptr};
const char* STORAGE_CLASS[] = {"extern", "export", "static", nullptr};
// Values which denote the beginning of a number.
const char NUMBERS[] = "-0123456789";
const char* OPERATORS[] = {
    "+", "-", "*", "/", "mod", "&", "|", "^", "&&", "||", "<<", ">>", "<", ">",
    "<=", ">=", "!=", "==", "!", "-", "~", nullptr
};
const char SINGLES[] = "(){},;";
const char SYMBOLS[] = "!-*&~+-/|^<>=(){}[],;";

int strinstrs(const std::string& str, const char** strs) {
    for (int i = 0; strs[i] != nullptr; i++) {
        if (str == strs[i])
            return i;
    }
    return -1;
}

TokenType determine_token_type(const std::string& string) {
    // Determine token type.

    // Collect single-char tokens.
    if (string.length() == 1) {
        if (string[0] == ',')
            return TokenType::COMMA;
        if (string[0] == ';')
            return TokenType::SEMICOLON;

        if (strchr(BRACKETS, string[0]))
            return TokenType::BRACKET;
    }

    // Parse the various types of keywords.

    // Declaration storage_class.
    if (strinstrs(string, STORAGE_CLASS) != -1)
        return TokenType::STORAGE_CLASS;
    // Other keywords.
    if (strinstrs(string, KEYWORDS) != -1)
        return TokenType::KEYWORD;
    // Then operators...
    if (strinstrs(string, OPERATORS) >= 0)
        return TokenType::OPERATOR;
    // Types...
    if (get_type_from_str(string) != -1)
        return TokenType::TYPE;
    // Number constants.
    if (strchr(NUMBERS, string[0]))
        return TokenType::INT;

    return TokenType::IDENTIFIER;
}

bool read_token(std::istream& infile, Token& token) {
    bool alpha_mode;

    while (1) {
        char next_char = infile.get();

        if (infile.eof()) {
            if (token.string.length() == 0)
                return true;
            break;
        }

        if (token.string.length() == 0) {
            // Ignore leading whitespace.
            if (isspace(next_char))
                continue;

            // Now check for single-char tokens, such as BRACKETS, ",", and ";".
            if (strchr(SINGLES, next_char) != NULL) {
                token.string += next_char;
                break;
            }

            // Check for negative signs.
            if (next_char == '-' and strchr(NUMBERS, infile.peek())) {
                token.string += '-';
                next_char = infile.get();
            }

            // If the first character is a symbol, then it can be delimited
            // by text (and vice-versa). This means "a+b" is valid, in
            // addition to "a + b".
            alpha_mode = strchr(SYMBOLS, next_char) != NULL;
        }

        // Whitespace delimits all tokens.
        if (isspace(next_char))
            break;

        if ((strchr(SYMBOLS, next_char) == NULL) == alpha_mode) {
            infile.unget();
            break;
        }

        // If the token did not end, append to the raw string and continue.
        token.string += next_char;
    }

    token.determine_type();

    return false;
}
