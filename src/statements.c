#include <stdio.h>
#include <inttypes.h>

#include "parser.h"
#include "statements.h"
#include "varray.h"

void fprint_value(FILE* out, Value* val) {
    if (!val->is_const)
        fprintf(out, "%%%" PRIu64, val->local_id);
    else if (val->is_signed)
        fprintf(out, "%" PRIi64, val->const_signed);
    else
        fprintf(out, "%" PRIu64, val->const_unsigned);
}

// Convert a statement structure into valid textual IR.
void fprint_statement(FILE* out, Statement* statement) {
    switch (statement->type) {
    case OPERATION: {
        Operation* op = statement;
        fprintf(out, "    %s %%%" PRIu64 " = ", TYPE[op->var_type], op->dest);
        switch (op->type) {
        case NOT: case NEGATE: case COMPLEMENT: case ADDRESS: case DEREFERENCE:
            fprintf(out, "%s%%%" PRIu64 ";\n", OPERATOR[op->type], op->lhs);
            break; // unops
        case ASSIGN:
            fprint_value(out, &op->rhs);
            fputs(";\n", out);
            break; //assign
        default:
            fprintf(out, "%%%" PRIu64 " %s ", op->lhs, OPERATOR[op->type]);
            fprint_value(out, &op->rhs);
            fputs(";\n", out);
            break; // binops
        }
    } break;
    case READ: {
        Read* read = statement;
        fprintf(out, "    %s %%%" PRIu64 " = %s;\n", TYPE[read->var_type], read->dest, read->src);
    } break;
    case WRITE:
        fprintf(out, "    %s = %%%" PRIu64 ";\n", ((Write*) statement)->dest, ((Write*) statement)->src);
        break;
    case JUMP:
        fprintf(out, "    jmp %s;\n", ((Jump*) statement)->label);
        break;
    case RETURN: {
        Return* ret = statement;
        fprintf(out, "    return %s%" PRIu64 ";\n", ret->val.is_const ? "" : "%", ret->val.const_unsigned);
    } break;
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
        Function* func = (Function*) declaration;

        fputc('(', out);

        if (func->parameter_count >= 1) {
            fprintf(out, "%s", TYPE[func->parameter_types[0]]);
            for (int i = 1; i < func->parameter_count; i++)
                fprintf(out, ", %s", TYPE[func->parameter_types[i]]);
        }

        fputs(") {\n", out);

        // Print statements using basic blocks, since this is the "optimized"
        // version of the master statement list.
        for (int i = 0; i < va_len(func->basic_blocks); i++) {
            if (func->basic_blocks[i].label != NULL)
                fprintf(out, "  @%s:\n", func->basic_blocks[i].label);

            for (Statement* state = func->basic_blocks[i].first; state != NULL; state = state->next)
                fprint_statement(out, state);
        }
        fputs("}\n", out);
    } else {
        fputs(";\n", out);
    }
}

void free_statement(Statement* statement) {
    switch (statement->type) {
    case READ: free(((Read*) statement)->src); break;
    case WRITE: free(((Write*) statement)->dest); break;
    case JUMP: free(((Jump*) statement)->label); break;
    case LABEL: free(((Label*) statement)->identifier); break;
    }
    free(statement);
}

void free_declaration(Declaration* declaration) {
    if (declaration->is_fn) {
        Function* func = (Function*) declaration;

        for (int i = 0; i < va_len(func->statements); i++)
            free_statement(func->statements[i]);
        for (int i = 0; i < va_len(func->locals); i++) {
            if (func->locals[i] != NULL)
                va_free(func->locals[i]->references);
        }

        free(func->parameter_types);
        va_free(func->basic_blocks);
        va_free(func->statements);
        va_free_contents(func->locals);
    }

    free(declaration->identifier);
    va_free_contents(declaration->traits);
    free(declaration);
}
