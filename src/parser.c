#define _GNU_SOURCE 1
#include <stdio.h>
#include <inttypes.h>

#include "exception.h"
#include "statements.h"
#include "varray.h"

#define SYMBOLS "!-*&~+-/|^<>=(){}[],;"
#define NUMBERS "1234567890"

const char* OPERATOR[] = { "=", // = is a dummy value.
    "+", "-", "*", "/", "mod", "&", "|", "^", "&&", "||", "<<", ">>", "<", ">",
    "<=", ">=", "!=", "==", "!", "-", "~", NULL
};
const char* TYPE[] = {"void", "u8", "u16", "u32", "u64", "i8", "i16", "i32", "i64", "f32", "f64", "p", NULL};
const char* STORAGE_CLASS[] = {"static", "extern", "export", NULL};

int strinstrs(const char* str, const char** strs) {
    for (int i = 0; strs[i] != NULL; i++)
        if (strcmp(str, strs[i]) == 0)
            return i;
    return -1;
}

int fpeek(FILE* f) {
    int c = getc(f);
    ungetc(c, f);
    return c;
}

void fdebugs(FILE* f) {
    long p = ftell(f);
    char* s = NULL;
    size_t n = 0;
    getline(&s, &n, f);
    fprintf(stderr, "DEBUG: %s", s);
    free(s);
    fseek(f, p, SEEK_SET);
}

Statement* fget_statement(FILE* infile) {
    char* first_token;

    fscanf(infile, "%ms ", &first_token);
    if (strinstrs(first_token, TYPE) != -1) {
        uint64_t dest = 255;
        fscanf(infile, "%%%" PRIu64 " = ", &dest);

        // If the first token is a type, this is either an operation, or a read.
        // Distiguishing reads is extremely simple; if the token after the '='
        // is an identifier (does not start with '%'), the statement is a read.

        // Check for unops early.
        char unop = 0;
        if (strchr(SYMBOLS, fpeek(infile)) != NULL)
            unop = fgetc(infile);

        if (fpeek(infile) == '%') {
            Operation* op = malloc(sizeof(Operation));
            op->statement.type = OPERATION;
            op->var_type = strinstrs(first_token, TYPE);
            op->dest = dest;

            // This is either an assignment or an operation.
            if (fpeek(infile) == ';') {
                // Either an assignment or a unop.
                if (unop) {
                    switch (unop) {
                    case '-':
                        op->type = NEGATE;
                        break;
                    case '!':
                        op->type = NOT;
                        break;
                    case '~':
                        op->type = COMPLEMENT;
                        break;
                    }
                    fscanf(infile, " %%%" PRIu64 ";", &op->lhs);
                } else {
                    op->type = ASSIGN;
                    // Parse the assignment value.
                    // In this case, we know it is a variable copy/cast.
                    fgetc(infile);
                    op->rhs.is_const = false;
                    fscanf(infile, " %%%" PRIu64 ";", &op->rhs.local_id);
                }
            } else {
                // Parse a binop.
                char* raw_operation;

                fscanf(infile, " %%%" PRIu64 " ", &op->lhs);
                fscanf(infile, " %ms ", &raw_operation);
                op->type = strinstrs(raw_operation, OPERATOR);
                if (op->type == -1)
                    error("Unknown operator \"%s\".", raw_operation);
                free(raw_operation);
                if (fpeek(infile) == '%') {
                    op->rhs.is_const = false;
                    fscanf(infile, "%%%" PRIu64 ";", &op->rhs.local_id);
                } else {
                    op->rhs.is_const = true;
                    if (fpeek(infile) == '-') {
                        fscanf(infile, "%" PRIi64 ";", &op->rhs.const_signed);
                    } else {
                        fscanf(infile, "%" PRIu64 ";", &op->rhs.const_unsigned);
                    }
                }
            }

            free(first_token);
            return &op->statement;
        } else if (strchr(NUMBERS, fpeek(infile))) {
            Operation* op = malloc(sizeof(Operation));
            op->statement.type = OPERATION;
            op->var_type = strinstrs(first_token, TYPE);
            op->dest = dest;
            op->type = ASSIGN;

            op->rhs.is_const = true;
            if (fpeek(infile) == '-') {
                fscanf(infile, "%" PRIi64 ";", &op->rhs.const_signed);
            } else {
                fscanf(infile, "%" PRIu64 ";", &op->rhs.const_unsigned);
            }

            free(first_token);
            return &op->statement;
        } else {
            Read* rd = malloc(sizeof(Read));
            rd->statement.type = READ;
            rd->var_type = strinstrs(first_token, TYPE);
            rd->dest = dest;
            fscanf(infile, " %m[^; \n] ;", &rd->src);

            free(first_token);
            return &rd->statement;
        }
    } else if (first_token[0] == '@') {
        Label* lab = malloc(sizeof(Label));
        lab->statement.type = LABEL;

        sscanf(first_token, "@%m[^: \n]:", &lab->identifier);

        free(first_token);
        return &lab->statement;
    } else if (strcmp(first_token, "return") == 0) {
        Return* ret = malloc(sizeof(Return));
        ret->statement.type = RETURN;

        if (fpeek(infile) == '%') {
            ret->val.is_const = false;
            fscanf(infile, "%%%" PRIu64 ";", &ret->val.local_id);
        } else {
            ret->val.is_const = true;
            if (fpeek(infile) == '-') {
                fscanf(infile, "%" PRIi64 ";", &ret->val.const_signed);
            } else {
                fscanf(infile, "%" PRIu64 ";", &ret->val.const_unsigned);
            }
        }

        free(first_token);
        return &ret->statement;
    } else if (strcmp(first_token, "jmp") == 0) {
        Jump* jmp = malloc(sizeof(Jump));
        jmp->statement.type = JUMP;

        fscanf(infile, " %m[^; \n] ;", &jmp->label);

        free(first_token);
        return &jmp->statement;
    } else {
        Write* wrt = malloc(sizeof(Write));
        wrt->statement.type = WRITE;
        wrt->dest = first_token;
        fscanf(infile, " = %%%" PRIu64 ";", &wrt->src);
        return &wrt->statement;
    }
}

Declaration* fget_declaration(FILE* infile) {
    char* storage_class;
    char* decl_type;
    char* var_type;
    char** trait_list = va_new(0);
    char* identifier;
    char** parameter_types = va_new(0);
    Statement** statement_block;

    fscanf(infile, " %ms %ms %ms ", &storage_class, &decl_type, &var_type);

    // Parse trait list (if present).
    char* next_string;
    fscanf(infile, " %m[^; ] ", &next_string);
    if (strcmp(next_string, "[[") == 0) {
        free(next_string);
        while (1) {
            fscanf(infile, " %ms ", &next_string);
            if (strcmp(next_string, "]]") == 0)
                break;
            va_append(trait_list, next_string);
        }
        free(next_string);
        fscanf(infile, " %m[^;( \n] ", &next_string);
    }

    // Collect name.
    identifier = next_string;

    // Parse parameters.
    if ((strcmp(storage_class, "export") == 0 || strcmp(storage_class, "static") == 0) && strcmp(decl_type, "fn") == 0) {
        if (getc(infile) != '(')
            fatal("Expected ( to begin function parameter list of \"%s\".", identifier);
        if (fpeek(infile) != ')') {
            while (fpeek(infile) != ')') {
                char* parameter_type;
                fscanf(infile, " %m[^,)]", &parameter_type);
                va_append(parameter_types, parameter_type);
                char next_char = fpeek(infile);
                if (next_char == ',')
                    fgetc(infile);
                else if (next_char != ')')
                    fatal("Unexpected character '%c' after function parameter of type \"%s\" in \"%s\"", next_char, parameter_type, identifier);
            }
        }
        char open_brace;
        fscanf(infile, ") %c ", &open_brace);
        if (open_brace != '{')
            fatal("Declaration of function \"%s\" missing opening brace ({).", identifier);

        statement_block = va_new(0);
        while (fpeek(infile) != '}') {
            Statement* statement = fget_statement(infile);
            fscanf(infile, " ");
            va_append(statement_block, statement);
        }
        fgetc(infile);
    } else if (getc(infile) != ';')
        fatal("Declaration of %s \"%s\" missing closing semicolon (;).", strcmp(decl_type, "fn") == 0 ? "function" : "variable", identifier);

    Declaration* decl = malloc(sizeof(Declaration));
    decl->storage_class = strinstrs(storage_class, STORAGE_CLASS);
    decl->identifier = identifier;
    decl->traits = trait_list;
    decl->type = strinstrs(var_type, TYPE);
    decl->is_fn = strcmp(decl_type, "fn") == 0;
    if (decl->is_fn) {
        decl = realloc(decl, sizeof(Function));
        Function* function = (Function*) decl;
        function->parameter_count = va_len(parameter_types);
        function->parameter_types = malloc(va_len(parameter_types) * sizeof(*function->parameter_types));
        for (int i = 0; i < va_len(parameter_types); i++) {
            function->parameter_types[i] = strinstrs(parameter_types[i], TYPE);
        }
        function->statements = statement_block;
    }

    free(storage_class);
    free(decl_type);
    free(var_type);
    va_free_contents(parameter_types);

    return decl;
}

Declaration** fparse_textual_ir(FILE* infile) {
    Declaration** decl_list = va_new(0);
    while (fscanf(infile, " "), !feof(infile)) {
        Declaration* decl = fget_declaration(infile);
        va_append(decl_list, decl);
    }
    return decl_list;
}
