#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef enum TokenType {
    TK_NONE,
    TK_COMMENT,
    TK_OPERATOR,
    TK_TYPE,
    TK_IDENTIFIER,
    TK_TRAIT,
    TK_BRACKET,
    TK_COMMA,
    TK_SEMICOLON,
    TK_INT,
    TK_FLOAT,
} TokenType;

typedef struct Token {
    TokenType type;
    char* string;
    unsigned long long value;
} Token;

typedef struct Context {
    size_t cur_token;
    size_t token_cnt;
    Token** token_list;
} Context;

void free_context(Context* context);
Context* get_context(FILE* input);
Token* next_token(Context* context);
Token* peek_token(Context* context);
bool expect_token(Context* context, char* expect);
size_t remaining_tokens(Context* context);
void skip_token(Context* context);