#include "exception.h"
#include "optimizer.h"
#include "statements.h"
#include "varray.h"

struct OptimizeOption {
    const char* name;
    bool* flag;
};

bool remove_unused = true;

const struct OptimizeOption optimization_options[] = {
    {"remove-unused", &remove_unused},
    {NULL, NULL}
};

void parse_opt_flag(const char* arg) {
    bool new_val = true;
    if (strncmp(arg, "no-", 3) == 0) {
        new_val = false;
        arg += 3;
    }
    for (int i = 0; optimization_options[i].name != NULL; i++) {
        if (strcmp(optimization_options[i].name, arg) == 0) {
            *optimization_options[i].flag = new_val;
            return;
        }
    }
    error("Optimization option \"%s\" not found.", arg);
}

void count_block_references(Function* func) {
    for (int i = 0; i < va_len(func->basic_blocks); i++)
        func->basic_blocks[i].ref_count = 0;
    for (int i = 0; i < va_len(func->basic_blocks); i++) {
        for (int j = 0; j < va_len(func->basic_blocks[i].statements); j++) {
            Statement* this_statement = func->basic_blocks[i].statements[j];

            if (this_statement->type == JUMP) {
                Jump* this_jump = ((Jump*) this_statement);

                for (int k = 0; k < va_len(func->basic_blocks); k++) {
                    if (func->basic_blocks[k].label == NULL)
                        continue;
                    if (strcmp(this_jump->label, func->basic_blocks[k].label) == 0) {
                        func->basic_blocks[k].ref_count += 1;
                        goto found_label;
                    }
                }
                error("Jump references nonexistant label \"%s\"", this_jump->label);
            found_label:
            }
        }
    }
}

void generate_basic_blocks(Function* func) {
    func->basic_blocks = va_new(sizeof(BasicBlock));
    func->basic_blocks->label = NULL;
    func->basic_blocks->statements = va_new(0);

    for (int i = 0; i < va_len(func->statements); i++) {
        uint8_t statement_type = func->statements[i]->type;
        BasicBlock* last_block = &func->basic_blocks[va_len(func->basic_blocks) - 1];
        // Labels begin new basic blocks, unless the current block is empty and
        // has no label!
        if (statement_type == LABEL) {
            if (last_block->label == NULL && va_len(last_block->statements) == 0) {
                last_block->label = ((Label*) func->statements[i])->identifier;
            } else {
                va_expand(&func->basic_blocks, sizeof(BasicBlock));
                BasicBlock* new_block = &func->basic_blocks[va_len(func->basic_blocks) - 1];
                new_block->label = ((Label*) func->statements[i])->identifier;
                new_block->ref_count = 0;
                new_block->statements = va_new(0);
                error("Label \"%s\" is not followed by a jump; implicit fallthroughs are not allowed.", new_block->label);
            }
        } else if (statement_type == JUMP || statement_type == RETURN) {
            va_append(last_block->statements, func->statements[i]);
            for (i++; i < va_len(func->statements); i++) {
                if (func->statements[i]->type != LABEL) {
                    error("Statement following a jump must be a label.");
                } else {
                    va_expand(&func->basic_blocks, sizeof(BasicBlock));
                    BasicBlock* new_block = &func->basic_blocks[va_len(func->basic_blocks) - 1];
                    new_block->label = ((Label*) func->statements[i])->identifier;
                    new_block->ref_count = 0;
                    new_block->statements = va_new(0);
                    break;
                }
            }
        } else {
            va_append(last_block->statements, func->statements[i]);
        }
    }

    count_block_references(func);
}

void remove_unused_blocks(Function* func) {
    for (int i = 1; i < va_len(func->basic_blocks); i++) {
        if (func->basic_blocks[i].ref_count == 0) {
            va_free(func->basic_blocks[i].statements);
            va_remove(func->basic_blocks, i);
            i -= 1; // Handle the change in size by offsetting i.
        }
    }
}

void remove_unused_fallthroughs(Function* func) {
    for (int i = 1; i < va_len(func->basic_blocks); i++) {
        if (func->basic_blocks[i].ref_count == 1
            && va_last(func->basic_blocks[i - 1].statements)->type == JUMP
            && strcmp(((Jump*) va_last(func->basic_blocks[i - 1].statements))->label, func->basic_blocks[i].label) == 0)
        {
            Statement** old_statements = func->basic_blocks[i].statements;
            size_t prev_len = va_len(func->basic_blocks[i - 1].statements);
            va_expand(&func->basic_blocks[i - 1].statements, va_size(old_statements) - sizeof(Statement*));
            memcpy(func->basic_blocks[i - 1].statements + prev_len - 1, old_statements, va_size(old_statements));

            va_free(old_statements);
            va_remove(func->basic_blocks, i);
            i -= 1; // Handle the change in size by offsetting i.
        }
    }
}

void optimize_ir(Declaration** decls) {
    // Remove unused basic blocks.
    if (remove_unused) {
        for (int i = 0; i < va_len(decls); i++)
            if (decls[i]->is_fn) {
                remove_unused_blocks((Function*) decls[i]);
                count_block_references((Function*) decls[i]);
                remove_unused_fallthroughs((Function*) decls[i]);
            }
    }
}
