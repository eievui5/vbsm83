#pragma once

#include <stdio.h>

enum TokenType {
    TK_NONE,
    TK_COMMENT,
    TK_OPERATOR,
    TK_TYPE,
    TK_IDENTIFIER,
};

struct token {
    enum TokenType type;
    char* string;
};

struct token* get_token(FILE* input);
void free_token(struct token* self);