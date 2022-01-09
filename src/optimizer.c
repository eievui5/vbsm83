#include "exception.h"
#include "optimizer.h"
#include "parser.h"
#include "statements.h"
#include "varray.h"

struct OptimizeOption {
    const char* name;
    bool* flag;
    const char* desc;
};

bool remove_unused = true;

const struct OptimizeOption optimization_options[] = {
    {"remove-unused", &remove_unused, "Remove unused blocks and fallthroughs."},
    {NULL}
};

void print_opt_help() {
    puts("Optimization options:\nPrefix an option with \"no-\" to disable it.");
    for (int i = 0; optimization_options[i].name != NULL; i++)
        printf("  -f%s %44s\n", optimization_options[i].name, optimization_options[i].desc);
}

void parse_opt_flag(const char* arg) {
    static bool displayed_help = false;
    bool new_val = true;

    if (strncmp(arg, "no-", 3) == 0) {
        new_val = false;
        arg += 3;
    }
    for (int i = 0; optimization_options[i].name != NULL; i++) {
        if (strequ(optimization_options[i].name, arg)) {
            *optimization_options[i].flag = new_val;
            return;
        }
    }

    error("Optimization option \"%s\" not found.", arg);
    // Only print help once if an error occurs.
    if (!displayed_help) {
        print_opt_help();
        displayed_help = true;
    }
}

void count_block_references(Function* func) {
    for (int i = 0; i < va_len(func->basic_blocks); i++)
        func->basic_blocks[i].ref_count = 0;
    for (int i = 0; i < va_len(func->basic_blocks); i++) {
        for (Statement* this_state = func->basic_blocks[i].first; this_state != NULL; this_state = this_state->next) {
            if (this_state->type == JUMP) {
                Jump* this_jump = ((Jump*) this_state);

                for (int k = 0; k < va_len(func->basic_blocks); k++) {
                    if (func->basic_blocks[k].label == NULL)
                        continue;
                    if (strequ(this_jump->label, func->basic_blocks[k].label)) {
                        func->basic_blocks[k].ref_count += 1;
                        goto found_label;
                    }
                }
                error("Jump references nonexistant label \"%s\"", this_jump->label);
            found_label:
                continue;
            }
        }
    }
}

void generate_local_vars(Function* func) {
    func->locals = va_new(func->parameter_count * sizeof(LocalVar*));

    // Begin with parameters
    for (int i = 0; i < func->parameter_count; i++) {
        func->locals[i] = malloc(sizeof(LocalVar));
        func->locals[i]->references = va_new(0);
    }

    // Collect locals.
    for (int i = 0; i < va_len(func->basic_blocks); i++) {
        for (Statement* this_state = func->basic_blocks[i].first; this_state != NULL; this_state = this_state->next) {
            size_t new_index;
            uint8_t type;

            switch (this_state->type) {
            case READ:
                new_index = ((Read*) this_state)->dest;
                type = ((Read*) this_state)->var_type;
                break;
            case OPERATION:
                new_index = ((Operation*) this_state)->dest;
                type = ((Operation*) this_state)->var_type;
                break;
            default:
                goto super_continue;
            }

            size_t prev_len = va_len(func->locals);
            if (prev_len <= new_index) {
                va_resize(&func->locals, (new_index + 1) * sizeof(LocalVar*));
                // Null new entries.
                memset(func->locals + prev_len, 0, (new_index + 1 - prev_len) * sizeof(LocalVar*));
            }
            if (func->locals[new_index] != NULL)
                fatal("Local variable %%%zu in %s has been assigned twice.", new_index, func->declaration.identifier);
            func->locals[new_index] = malloc(sizeof(LocalVar));
            func->locals[new_index]->type = type;
            func->locals[new_index]->origin = this_state;
            func->locals[new_index]->references = va_new(0);
        super_continue:
            continue;
        }
    }
}

static void append_to_block(BasicBlock* bb, Statement* st) {
    if (bb->first == NULL) {
        // If the list is empty set st to both the first and final entry...
        bb->first = st;
        bb->final = st;
        // ...with no links.
        st->last = NULL;
        st->next = NULL;
    } else {
        // Otherwise set st to the last entry, update the old last entry's next
        // link to st, and set st's last link to the old last.
        st->last = bb->final;
        bb->final->next = st;
        bb->final = st;
        st->next = NULL;
    }
}

static inline void init_block(BasicBlock* bb, char* label) {
    bb->label = label;
    bb->ref_count = 0;
    bb->first = NULL;
    bb->final = NULL;
}

void generate_basic_blocks(Function* func) {
    func->basic_blocks = va_new(sizeof(BasicBlock));
    init_block(func->basic_blocks, NULL);

    for (int i = 0; i < va_len(func->statements); i++) {
        uint8_t statement_type = func->statements[i]->type;
        BasicBlock* last_block = &func->basic_blocks[va_len(func->basic_blocks) - 1];
        // Labels begin new basic blocks, unless the current block is empty and
        // has no label!
        if (statement_type == LABEL) {
            if (last_block->label == NULL && last_block->first == NULL) {
                last_block->label = ((Label*) func->statements[i])->identifier;
            } else {
                va_expand(&func->basic_blocks, sizeof(BasicBlock));
                BasicBlock* new_block = &func->basic_blocks[va_len(func->basic_blocks) - 1];
                init_block(new_block, ((Label*) func->statements[i])->identifier);
                error("Label \"%s\" is not followed by a jump; implicit fallthroughs are not allowed.", new_block->label);
            }
        } else if (statement_type == JUMP || statement_type == RETURN) {
            append_to_block(last_block, func->statements[i]);
            for (i++; i < va_len(func->statements); i++) {
                if (func->statements[i]->type != LABEL) {
                    error("Statement following a jump must be a label.");
                } else {
                    va_expand(&func->basic_blocks, sizeof(BasicBlock));
                    init_block(&func->basic_blocks[va_len(func->basic_blocks) - 1],
                               ((Label*) func->statements[i])->identifier);
                    break;
                }
            }
        } else {
            append_to_block(last_block, func->statements[i]);
        }
    }

    count_block_references(func);
}

// Removes blocks which are never referenced.
// This should be called multiple times to handle any changes in the program
// flow.
void remove_unused_blocks(Function* func) {
    for (int i = 1; i < va_len(func->basic_blocks); i++) {
        // If the refcount of a block is 0, then there is no way it can be
        // reached.
        if (func->basic_blocks[i].ref_count == 0) {
            va_remove(func->basic_blocks, i);
            i -= 1; // Handle the change in size by offsetting i.
        }
    }
    // After removing code, block references must be updated.
    count_block_references(func);
}

// Removes any unneccessary fallthroughs to unify basic blocks, allowing for
// greater optimization potential.
void remove_unused_fallthroughs(Function* func) {
    for (int i = 1; i < va_len(func->basic_blocks); i++) {
        // If a block is only referenced once, and only by the final statement
        // of the block before it, then the two blocks can be unified into one,
        // and the jump can be removed entirely.
        if (func->basic_blocks[i].ref_count == 1
            && func->basic_blocks[i - 1].final->type == JUMP
            && strequ(((Jump*) func->basic_blocks[i - 1].final)->label, func->basic_blocks[i].label))
        {
            for (Statement* state = func->basic_blocks[i].final->last; state != NULL;) {
                Statement* this_state = state;
                state = state->last;
                append_to_block(&func->basic_blocks[i - 1], this_state);
            }

            va_remove(func->basic_blocks, i);
            i -= 1; // Handle the change in size by offsetting i.
        }
    }
}

// Run various optimizations according to the user's options.
void optimize_ir(Declaration** decls) {
    // Remove unused basic blocks.
    if (remove_unused) {
        for (int i = 0; i < va_len(decls); i++)
            if (decls[i]->is_fn) {
                Function* func = (Function*) decls[i];
                remove_unused_blocks(func);
                remove_unused_fallthroughs(func);
            }
    }
}
