#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef enum VariableType {
    VT_U8 = 0, VT_U16, VT_U32, VT_U64,
    VT_I8, VT_I16, VT_I32, VT_I64,
    VT_F32, VT_F64, VT_PTR, VT_FARPTR,
    VT_VOID,
} VariableType;

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
    TK_KEYWORD,
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

extern const char COMMENT[];
extern const char BRACKETS[];
extern const char* KEYWORDS[];
extern const char NUMBERS[];
extern const char* OPERATORS[];
extern const char SINGLES[];
extern const char SYMBOLS[];
extern const char* TYPES[];
extern const char WHITESPACE[];

bool expect_token(Context* context, char* expect);
void free_context(Context* context);
Context* get_context(FILE* input);
Token* next_token(Context* context);
Token* peek_token(Context* context);
size_t remaining_tokens(Context* context);
Token* seek_token(Context* context, int off);
void skip_token(Context* context);
int strinstrs(const char* str, const char** strs);
