#include <stdio.h>
#include <inttypes.h>

#include "parser.h"
#include "statements.h"
#include "varray.h"

// Convert a statement structure into valid textual IR.
void fprint_statement(FILE* out, Statement* statement) {
    switch (statement->type) {
    case OPERATION:
        fprintf(out, "    %s %%%" PRIu64 " = ", TYPE[((Operation*) statement)->var_type], ((Operation*) statement)->dest);
        switch (((Operation*) statement)->type) {
        case NOT: case NEGATE: case COMPLEMENT:
            fprintf(out, "%s%%%" PRIu64 ";\n", OPERATOR[((Operation*) statement)->type], ((Operation*) statement)->lhs);
            break; // unops
        case ASSIGN:
            fprintf(out, "%s%" PRIu64 ";\n", ((Operation*) statement)->rhs.is_const ? "" : "%",
                    ((Operation*) statement)->rhs.const_unsigned);
            break; //assign
        default:
            fprintf(out, "%%%" PRIu64 " %s %s%" PRIu64 ";\n", ((Operation*) statement)->lhs, OPERATOR[((Operation*) statement)->type],
                    ((Operation*) statement)->rhs.is_const ? "" : "%", ((Operation*) statement)->rhs.const_unsigned);
            break; // binops
        }
        break;
    case READ:
        fprintf(out, "    %s %%%" PRIu64 " = %s;\n", TYPE[((Read*) statement)->var_type], ((Read*) statement)->dest,
                ((Read*) statement)->src);
        break;
    case WRITE:
        fprintf(out, "    %s = %%%" PRIu64 ";\n", ((Write*) statement)->dest, ((Write*) statement)->src);
        break;
    case JUMP:
        fprintf(out, "    jmp %s;\n", ((Jump*) statement)->label);
        break;
    case RETURN:
        fprintf(out, "    return %s%" PRIu64 ";\n", ((Return*) statement)->val.is_const ? "" : "%",
                ((Return*) statement)->val.const_unsigned);
        break;
    case LABEL:
        fprintf(out, "  @%s:\n", ((Label*) statement)->identifier);
        break;
    }
}

// Convert a declaration structure into valid textual IR. If a function is
// encountered, its statements will be printed as well.
void fprint_declaration(FILE* out, Declaration* declaration) {
    fprintf(out, "%s %s %s [[ ",
            STORAGE_CLASS[declaration->storage_class],
            declaration->is_fn ? "fn" : "var",
            TYPE[declaration->type]);
    for (int i = 0; i < va_len(declaration->traits); i++)
        fprintf(out, "%s ", declaration->traits[i]);
    fprintf(out, "]] %s", declaration->identifier);
    if (declaration->is_fn) {
        Function* function = (Function*) declaration;
        fputc('(', out);
        if (function->parameter_count >= 1) {
            fprintf(out, "%s", TYPE[function->parameter_types[0]]);
            for (int i = 1; i < function->parameter_count; i++)
                fprintf(out, ", %s", TYPE[function->parameter_types[i]]);
        }
        fputs(") {\n", out);
        for (int i = 0; i < va_len(function->statements); i++)
            fprint_statement(out, function->statements[i]);
        fputs("}\n", out);
    } else {
        fputs(";\n", out);
    }
}

void free_statement(Statement* statement) {
    switch (statement->type) {
    case OPERATION:
        break;
    case READ:
        free(((Read*) statement)->src);
        break;
    case WRITE:
        free(((Write*) statement)->dest);
        break;
    case JUMP:
        free(((Jump*) statement)->label);
        break;
    case RETURN:
        break;
    case LABEL:
        free(((Label*) statement)->identifier);
        break;
    }
    free(statement);
}

void free_declaration(Declaration* declaration) {
    free(declaration->identifier);
    va_free_contents(declaration->traits);
    if (declaration->is_fn) {
        Function* function = (Function*) declaration;
        free(function->parameter_types);
        for (int i = 0; i < va_len(function->statements); i++)
            free_statement(function->statements[i]);
        va_free(function->statements);
    }
    free(declaration);
}
