#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum VariableType {
    VOID,
    U8, U16, U32, U64,
    I8, I16, I32, I64,
    F32, F64,
    PTR,
};

/*
    {  "u8", 1},
    { "u16", 2},
    { "u32", 4},
    { "u64", 8},
    {  "i8", 1},
    { "i16", 2},
    { "i32", 4},
    { "i64", 8},
    { "f32", 4},
    { "f64", 8},
    {   "p", 2},
    {"void", 0},
*/

enum StatementType {
    OPERATION,
    READ,
    WRITE,
    JUMP,
    RETURN,
    LABEL,
    END_BLOCK = -1
};

enum OpType {
    ASSIGN,
    ADD, SUB, MUL, DIV, MOD,
    B_AND, B_OR, B_XOR, L_AND, L_OR,
    LSH, RSH,
    LESS, GREATER, LESS_EQU, GREATER_EQU, NOT_EQU, EQU,
    NOT, NEGATE, COMPLEMENT,
};

typedef enum StorageClass { STATIC, EXTERN, EXPORT } StorageClass;

typedef struct Statement { uint8_t type; } Statement;

// Used when both constants and locals are possible options.
typedef struct Value {
    bool is_const;
    union {
        uint64_t const_unsigned;
        int64_t  const_signed;
        uint64_t local_id;
    };
} Value;

// Global variables and functions.
typedef struct Declaration {
    uint8_t storage_class;
    char* identifier;
    char** traits;
    uint8_t type;
    bool is_fn; // True if this structure is part of a `Function`
} Declaration;

// Five statement types.
typedef struct Operation {
    Statement statement;
    uint8_t type;
    uint8_t var_type;
    uint64_t dest; // ID of destination variable.
    uint64_t lhs; // ID of source varible; may not be a constant.
    Value rhs; // Either a constant or a local ID. Ignored unless using binops.
} Operation;

typedef struct Read {
    Statement statement;
    uint8_t var_type;
    uint64_t dest;
    char* src;
} Read;

typedef struct Write {
    Statement statement;
    char* dest;
    uint64_t src;
} Write;

typedef struct Jump {
    Statement statement;
    char* label; // Could also be an ID or something.
} Jump;

typedef struct Return {
    Statement statement;
    Value val;
} Return;

typedef struct Label {
    Statement statement;
    char* identifier;
} Label;

// A simple list of statements, beginning with an optional label and ending with
// either a jump or return. Basic blocks may later be converted back into a
// plain statement list, if IR output is needed.
typedef struct BasicBlock {
    char* label; // May be NULL.
    Statement** statements;
    unsigned ref_count;
} BasicBlock;

// Functions can simply be treated as read-only global variables.
typedef struct Function {
    Declaration declaration;
    Statement** statements;
    size_t parameter_count;
    uint8_t* parameter_types;
    BasicBlock* basic_blocks;
} Function;

void fprint_statement(FILE* out, Statement* statement);
void fprint_declaration(FILE* out, Declaration* declaration);
void free_statement(Statement* statement);
void free_declaration(Declaration* declaration);
