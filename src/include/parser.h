#pragma once

#include <stdio.h>

typedef enum TokenType {
    TK_NONE,
    TK_COMMENT,
    TK_OPERATOR,
    TK_TYPE,
    TK_IDENTIFIER,
    TK_TRAIT,
} TokenType;

typedef struct Token {
    TokenType type;
    char* string;
} Token;

typedef struct Context {
    size_t token_cnt;
    Token** token_list;
} Context;

void free_context(Context* context);
Context* get_context(FILE* input);