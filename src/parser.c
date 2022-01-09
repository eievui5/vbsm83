#define _GNU_SOURCE 1
#include <stdio.h>
#include <inttypes.h>

#include "exception.h"
#include "optimizer.h"
#include "statements.h"
#include "varray.h"

#define SYMBOLS "!-*&~+-/|^<>=(){}[],;"
#define NUMBERS "1234567890"
#define WHITESPACE " \n\t"

const char* OPERATOR[] = { "=", // `=` is a dummy value.
    "+", "-", "*", "/", "mod", "&", "|", "^", "&&", "||", "<<", ">>", "<", ">",
    "<=", ">=", "!=", "==",
    // Unary operators are special cases, and should *not* expected from `strinstrs()`.
    "!", "-", "~", "&", "*", NULL
};
const char* TYPE[] = {"void", "u8", "u16", "u32", "u64", "i8", "i16", "i32", "i64", "f32", "f64", "p", NULL};
const char* STORAGE_CLASS[] = {"static", "extern", "export", NULL};

// Check for the occurance of a given string within a NULL-terminated array of
// strings. Returns -1 upon failure.
int strinstrs(const char* str, const char** strs) {
    for (int i = 0; strs[i] != NULL; i++)
        if (strequ(str, strs[i]))
            return i;
    return -1;
}

// Return the next character in a file without consuming it.
int fpeek(FILE* f) {
    int c = getc(f);
    ungetc(c, f);
    return c;
}

// Print the remaining characters in the current line of a file; for debugging.
void fdebugs(FILE* f) {
    long p = ftell(f);
    char* s = NULL;
    size_t n = 0;
    getline(&s, &n, f);
    fprintf(stderr, "DEBUG: %s", s);
    free(s);
    fseek(f, p, SEEK_SET);
}

// I can't use fscanf("%ms") because I need to support MacOS...
char* fmgetx(FILE* f, char* exclude) {
    fscanf(f, " "); // Skip leading whitespace.
    int i = 0;
    int len = 16;
    char* str = malloc(len);
    while (strchr(exclude, fpeek(f)) == NULL) {
        str[i++] = fgetc(f);
        if (len - 1 == i)
            str = realloc(str, len *= 2);
    }
    str[i] = 0;
    fscanf(f, " "); // Skip trailing whitespace.
    return str;
}

static inline char* fmgets(FILE* f) { return fmgetx(f, WHITESPACE); }

// Determines if the following value is a local variable, signed constant, or
// unsigned constant.
void fdetermine_const_value(FILE* infile, Value* val) {
    val->is_const = true;
    if (fpeek(infile) == '-') {
        val->is_signed = true;
        fscanf(infile, "%" PRIi64 ";", &val->const_signed);
    } else {
        val->is_signed = false;
        fscanf(infile, "%" PRIu64 ";", &val->const_unsigned);
    }
}

// Determines if the following value is a local variable, signed constant, or
// unsigned constant.
void fdetermine_value(FILE* infile, Value* val) {
    if (fpeek(infile) == '%') {
        val->is_const = false;
        fscanf(infile, "%%%" PRIu64 ";", &val->local_id);
    } else {
        fdetermine_const_value(infile, val);
    }
}

// Read a statement from an IR file.
Statement* fget_statement(FILE* infile) {
    char* first_token = fmgets(infile);

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
            fscanf(infile, " %%%" PRIu64 " ", &op->lhs);

            // This is either an assignment or an operation.
            if (fpeek(infile) == ';') {
                fgetc(infile);
                // Either an assignment or a unop.
                if (unop) {
                    switch (unop) {
                    case '-': op->type = NEGATE; break;
                    case '!': op->type = NOT; break;
                    case '~': op->type = COMPLEMENT; break;
                    case '&': op->type = ADDRESS; break;
                    case '*': op->type = DEREFERENCE; break;
                    }
                } else {
                    op->type = ASSIGN;
                    // Parse the assignment value.
                    // In this case, we know it is a variable copy/cast.
                    fgetc(infile);
                    op->rhs.is_const = false;
                    op->rhs.local_id = op->lhs;
                }
            } else {
                // Parse a binop.
                char* raw_operation = fmgets(infile);

                op->type = strinstrs(raw_operation, OPERATOR);
                free(raw_operation);

                fdetermine_value(infile, &op->rhs);
            }

            free(first_token);
            return &op->statement;
        } else if (strchr(NUMBERS, fpeek(infile))) {
            Operation* op = malloc(sizeof(Operation));
            op->statement.type = OPERATION;
            op->var_type = strinstrs(first_token, TYPE);
            op->dest = dest;
            op->type = ASSIGN;

            fdetermine_const_value(infile, &op->rhs);

            free(first_token);
            return &op->statement;
        } else {
            Read* rd = malloc(sizeof(Read));
            rd->statement.type = READ;
            rd->var_type = strinstrs(first_token, TYPE);
            rd->dest = dest;
            rd->src = fmgetx(infile, ";" WHITESPACE);
            if (fgetc(infile) != ';')
                error("Expected semicolon after read statement.");

            free(first_token);
            return &rd->statement;
        }
    } else if (first_token[0] == '@') {
        Label* lab = malloc(sizeof(Label));
        lab->statement.type = LABEL;

        memmove(first_token, first_token + 1, strlen(first_token) - 1);
        char* term = strchr(first_token, ':');
        if (term == NULL)
            fatal("Missing terminating colon in label declaration.");
        *term = 0;

        lab->identifier = first_token;

        return &lab->statement;
    } else if (strequ(first_token, "return")) {
        Return* ret = malloc(sizeof(Return));
        ret->statement.type = RETURN;

        fdetermine_value(infile, &ret->val);

        free(first_token);
        return &ret->statement;
    } else if (strequ(first_token, "jmp")) {
        Jump* jmp = malloc(sizeof(Jump));
        jmp->statement.type = JUMP;

        jmp->label = fmgetx(infile, ";" WHITESPACE);
        if (fgetc(infile) != ';')
            error("Expected semicolon after jump statement.");

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

// Read a declaration from an IR file. If a function is encountered, its
// statements will be read and stored.
Declaration* fget_declaration(FILE* infile) {
    char** trait_list = va_new(0);
    char* identifier;
    char** parameter_types = va_new(0);
    Statement** statement_block;

    char* storage_class = fmgets(infile);
    char* decl_type = fmgets(infile);
    char* var_type = fmgets(infile);

    // Parse trait list (if present).
    char* next_string = fmgetx(infile, ";" WHITESPACE);
    if (strequ(next_string, "[[")) {
        free(next_string);
        while (1) {
            next_string = fmgets(infile);
            if (strequ(next_string, "]]"))
                break;
            va_append(trait_list, next_string);
        }
        free(next_string);
        next_string = fmgetx(infile, ";(" WHITESPACE);
    }

    // Collect name.
    identifier = next_string;

    // Parse parameters.
    if ((strequ(storage_class, "export") || strequ(storage_class, "static")) && strequ(decl_type, "fn")) {
        if (getc(infile) != '(')
            fatal("Expected ( to begin function parameter list of \"%s\".", identifier);
        if (fpeek(infile) != ')') {
            while (fpeek(infile) != ')') {
                char* parameter_type = fmgetx(infile, ",)");
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
        fatal("Declaration of %s \"%s\" missing closing semicolon (;).", strequ(decl_type, "fn") ? "function" : "variable", identifier);

    Declaration* decl = malloc(sizeof(Declaration));
    decl->storage_class = strinstrs(storage_class, STORAGE_CLASS);
    decl->identifier = identifier;
    decl->traits = trait_list;
    decl->type = strinstrs(var_type, TYPE);
    decl->is_fn = strequ(decl_type, "fn");
    if (decl->is_fn) {
        decl = realloc(decl, sizeof(Function));
        Function* func = (Function*) decl;
        func->parameter_count = va_len(parameter_types);
        func->parameter_types = malloc(va_len(parameter_types) * sizeof(*func->parameter_types));
        for (int i = 0; i < va_len(parameter_types); i++) {
            func->parameter_types[i] = strinstrs(parameter_types[i], TYPE);
        }
        func->statements = statement_block;
        generate_basic_blocks(func);
        generate_local_vars(func);
    }

    free(storage_class);
    free(decl_type);
    free(var_type);
    va_free_contents(parameter_types);

    return decl;
}

// Parse an entire file, including all declarations and statements, and return
// a VArray of declarations.
Declaration** fparse_textual_ir(FILE* infile) {
    Declaration** decl_list = va_new(0);
    // Continue until the end of the file is reached.
    while (fscanf(infile, " "), !feof(infile)) {
        Declaration* decl = fget_declaration(infile);
        va_append(decl_list, decl);
    }
    return decl_list;
}
