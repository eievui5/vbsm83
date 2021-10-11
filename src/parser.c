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

char* append_char(char* str, char c, size_t i) {
    // Append this character to the end of the string.
    str[i++] = c;

    // If we've run out of room, reallocate the token string.
    if (i % 16 == 0) {
        str = realloc(str, i + 16);
    }
    str[i] = 0;

    return str;
}

TokenType get_token_type(char* token_str) {
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
Token* get_token(FILE* input) {
    bool is_symbol = false;
    Token* new_token = malloc(sizeof(Token));
    new_token->string = malloc(16); // Allocate in 16-byte chunks.
    memset(new_token->string, 0, 16);
    int next_char;
    size_t token_index = 0;

    // Parse out the token, making sure to differentiate between SYMBOLS and
    // labels.
    while (1) {
        // Get the next character.
        next_char = fgetc(input);

        // First, check for EOF.
        if (next_char == EOF) {
            // If this is the first char, return NULL.
            if (token_index == 0) {
                free(new_token);
                free(new_token->string);
                return NULL;
            } else {
                break;
            }
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
        new_token->string = append_char(new_token->string, next_char, token_index++);
    }

    // Determine token type.
    new_token->type = get_token_type(new_token->string);

    // Special handling for comments.
    if (new_token->type == TK_COMMENT) {
        // Copy the rest of the line when a comment appears. This allows the IR
        // to output comments into the assembly code.
        while (1) {
            next_char = fgetc(input);
            if (next_char == '\n' || next_char == EOF) {
                break;
            }

            new_token->string = append_char(new_token->string, next_char, token_index++);
        }
    }

    // If we don't know the token's type, we can assume it's an identifier
    // (sometimes).
    if (new_token->type == TK_NONE) {
        if (is_symbol) {
            // Throw an error; Unknown Symbol.
        } else {
            new_token->type = TK_IDENTIFIER;
        }
    }

    return new_token;
}

/* Clean up a token before allowing it to exit scope.
*/
void free_token(Token* self) {
    free(self->string);
    free(self);
}

/* Create a new context stucture using the given file.
*/
Context* get_context(FILE* input) {
    Context* context = malloc(sizeof(Context));
    context->token_cnt = 0;
    context->token_list = malloc(sizeof(Token*) * 16);

    while (1) {
        Token* next_tok = get_token(input);

        if (next_tok == NULL) {
            puts("Finished!");
            break;
        }

        printf("Cur tok: %li\n", context->token_cnt);
        context->token_list[context->token_cnt++] = next_tok;

        if (context->token_cnt % 16 == 0) {
            context->token_list = realloc(context->token_list, sizeof(Token*) * (context->token_cnt + 16));
        }
    }

    return context;
}

/* Clean up context before allowing it to exit scope.
*/
void free_context(Context* context) {
    for (int i = 0; i < context->token_cnt; i++) {
        free(context->token_list[i]);
    }
    free(context->token_list);
    free(context);
}