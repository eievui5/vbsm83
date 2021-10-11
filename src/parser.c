#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

static const char COMMENT[] = "//";
static const char* OPERATORS[] = {
    "!", "-", "*", "&", "~", "+", "/", "&", "|", "^", "&&", "||", "mod", "<<",
    ">>", "<", ">", "<=", ">=", "!=", "==",
    NULL
};
static const char SYMBOLS[] = "!-*&~+-/|^<>=(){}[]";
static const char* TYPES[] = {
    "u8", "u16", "u32", "u64", "i8", "i16", "i32", "i64", "f32", "f64", "p",
    "farp", "void", NULL
};
static const char WHITESPACE[] = " \n\t";

enum TokenType get_token_type(char* token_str) {
    // Determine token type.

    // Check for comments first
    if (strstr(token_str, COMMENT) != NULL) {
        return TK_COMMENT;
    }

    // Then operators...
    for (int i = 0; OPERATORS[i] != NULL; i++) {
        if (strcmp(token_str, OPERATORS[i]) == 0) {
            return TK_OPERATOR;
        }
    }

    // Types...
    for (int i = 0; TYPES[i] != NULL; i++) {
        if (strcmp(token_str, TYPES[i]) == 0) {
            return TK_TYPE;
        }
    }

    // Fallback.
    return TK_NONE;
}

/* Create a new token from a given file.
*/
struct token* get_token(FILE* input) {
    bool is_symbol = false;
    struct token* new_token = malloc(sizeof(struct token));
    new_token->string = malloc(16); // Allocate in 16-byte chunks.
    memset(new_token->string, 0, 16);
    int next_char;
    int token_index = 0;

    // Parse out the token, making sure to differentiate between SYMBOLS and
    // labels.
    while (1) {
        // Get the next character.
        next_char = fgetc(input);

        // First, check for EOF.
        if (next_char == EOF) {
            free(new_token);
            free(new_token->string);
            return NULL;
        }

        // The first character requires special logic to determine the token's
        // type.
        if (token_index == 0) {
            // Ignore leading whitespace.
            if (strchr(WHITESPACE, next_char) != NULL) {
                continue;
            }

            // If the first character is a symbol, then it can be delimited by
            // text (and vice-versa). This means "a+b" is valid, in addition to
            // "a + b".
            is_symbol = strchr(SYMBOLS, next_char) != NULL;
        }

        // whitespace delimits all tokens.
        // Match whether or not the char is a symbol to whether or not the token
        // is one.
        if (strchr(WHITESPACE, next_char) != NULL) {
            break;
        }
        if ((strchr(SYMBOLS, next_char) == NULL) == is_symbol) {
            fseek(input, -1, SEEK_CUR);
            break;
        }

        // Append this character to the end of the string.
        new_token->string[token_index++] = next_char;

        // If we've run out of room, reallocate the token string.
        if (token_index % 16 == 0) {
            new_token->string = realloc(new_token->string, token_index + 16);
            memset(&new_token->string[token_index], 0, 16);
        }
    }

    // Determine token type.
    new_token->type = get_token_type(new_token->string);

    // Special handling for comments.
    if (new_token->type == TK_COMMENT) {
        // Ignore the rest of the line when a comment appears.
        while (1) {
            next_char = fgetc(input);
            if (next_char == '\n' || next_char == EOF) {
                break;
            }
        }
    }
    if (new_token->type == TK_NONE) {
        if (is_symbol) {
            // Throw an error; Unknown Symbol.
        } else {
            new_token->type = TK_IDENTIFIER;
        }
    }

    return new_token;
}

/* Cleanup a token before allowing it to exit scope.
*/
void free_token(struct token* self) {
    free(self->string);
    free(self);
}