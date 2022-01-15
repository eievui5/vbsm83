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
bool fold_constants = true;

const struct OptimizeOption optimization_options[] = {
    {"remove-unused",  &remove_unused,  "Remove unused blocks and fallthroughs."},
    {"fold-constants", &fold_constants, "Evaluate constant operations and replace them with assignments."},
    {NULL}
};

// Print info about each of the possible optimization options.
void print_opt_help() {
    puts("Optimization options:\nPrefix an option with \"no-\" to disable it.");
    for (size_t i = 0; optimization_options[i].name; i++)
        printf("  -f%-16s %s\n", optimization_options[i].name, optimization_options[i].desc);
}

// Read a -f flag and enable or disable the corresponding option.
void parse_opt_flag(const char* arg) {
    static bool displayed_help = false;
    bool new_val = true;

    if (strncmp(arg, "no-", 3) == 0) {
        new_val = false;
        arg += 3;
    }
    for (size_t i = 0; optimization_options[i].name; i++) {
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

// Add a statement as the final element in a basic block.
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
    st->parent = bb;
}

// Remove a statement from a basic block
static void remove_from_block(BasicBlock* bb, Statement* st) {
    if (bb->first == st)
        bb->first = st->next;
    if (bb->final == st)
        bb->final = st->last;
    if (st->next)
        st->next->last = st->last;
    if (st->last)
        st->last->next = st->next;
}

// Initiallize members of a new basic block.
static void init_block(BasicBlock* bb, char* label) {
    bb->label = label;
    bb->ref_count = 0;
    bb->first = NULL;
    bb->final = NULL;
}

static void init_local(LocalVar** local, Statement* origin, uint8_t type) {
    *local = malloc(sizeof(LocalVar));
    LocalVar* this = *local;

    this->origin = origin;
    this->references = va_new(0);
    this->type = type;
    this->lifetime_start = 0;
    this->lifetime_end = 0;
    this->active_reg = 0;
    this->reg_reallocs = va_new(0);
}

// Check if a local variable has a known, constant value.
static bool is_local_const(LocalVar* local) {
    return local->origin && local->origin->type == OPERATION
           && ((Operation*) local->origin)->type == ASSIGN
           && ((Operation*) local->origin)->rhs.is_const;
}

// Get the constant value assigned to a local variable.
static inline Value* get_local_const(LocalVar* local) {
    return &((Operation*) local->origin)->rhs;
}

// Count each time that a basic block is referenced by a jump.
void count_block_references(Function* func) {
    for (size_t i = 0; i < va_len(func->basic_blocks); i++)
        func->basic_blocks[i].ref_count = 0;
    for (size_t i = 0; i < va_len(func->basic_blocks); i++) {
        for (Statement* this_state = func->basic_blocks[i].first; this_state; this_state = this_state->next) {
            if (this_state->type == JUMP) {
                Jump* this_jump = ((Jump*) this_state);

                for (size_t k = 0; k < va_len(func->basic_blocks); k++) {
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

// Count each time that a local variable is referenced in a function.
void count_local_references(Function* func) {
    for (size_t i = 0; i < va_len(func->basic_blocks); i++) {
        for (Statement* this_state = func->basic_blocks[i].first; this_state; this_state = this_state->next) {
            switch (this_state->type) {
            case OPERATION: {
                Operation* op = (Operation*) this_state;
                switch (op->type) {
                default:
                    if (!op->rhs.is_const)
                        va_append(func->locals[op->rhs.local_id]->references, &op->rhs.local_id);
                    // fallthrough
                case NOT: case NEGATE: case COMPLEMENT: case ADDRESS: case DEREFERENCE:
                    va_append(func->locals[op->lhs]->references, &op->lhs);
                    break; // unops
                case ASSIGN:
                    if (!op->rhs.is_const)
                        va_append(func->locals[op->rhs.local_id]->references, &op->rhs.local_id);
                    break; //assign
                }
            } break;
            case WRITE:
                va_append(get_local(func, ((Write*) this_state)->src)->references, &((Write*) this_state)->src);
                break;
            case RETURN: {
                Return* ret = (Return*) this_state;
                if (!ret->val.is_const)
                    va_append(get_local(func, ret->val.local_id)->references, &ret->val.local_id);
            } break;
            }
        }
    }
}

void generate_local_vars(Function* func) {
    func->locals = va_new(func->parameter_count * sizeof(LocalVar*));

    // Begin with parameters
    for (size_t i = 0; i < func->parameter_count; i++)
        init_local(&func->locals[i], NULL, func->parameter_types[i]);

    // Collect locals.
    for (size_t i = 0; i < va_len(func->basic_blocks); i++) {
        for (Statement* this_state = func->basic_blocks[i].first; this_state; this_state = this_state->next) {
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
                continue;
            }

            size_t prev_len = va_len(func->locals);

            if (prev_len <= new_index) {
                va_resize(&func->locals, (new_index + 1) * sizeof(LocalVar*));
                // Null new entries.
                memset(func->locals + prev_len, 0, (new_index + 1 - prev_len) * sizeof(LocalVar*));
            }

            if (func->locals[new_index])
                fatal("Local variable %%%zu in %s has been assigned twice.", new_index, func->declaration.identifier);

            init_local(&func->locals[new_index], this_state, type);
        }
    }

    count_local_references(func);
}

void generate_basic_blocks(Function* func) {
    func->basic_blocks = va_new(sizeof(BasicBlock));
    init_block(func->basic_blocks, NULL);

    for (size_t i = 0; i < va_len(func->statements); i++) {
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
                error("Label \"%s\" is not followed by a jump; implicit fallthroughs are not allowed.",
                      new_block->label);
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
    for (size_t i = 1; i < va_len(func->basic_blocks); i++) {
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
    for (size_t i = 1; i < va_len(func->basic_blocks); i++) {
        BasicBlock* this_block = &func->basic_blocks[i];
        BasicBlock* last_block = &func->basic_blocks[i - 1];

        // If a block is only referenced once, and only by the final statement
        // of the block before it, then the two blocks can be unified into one,
        // and the jump can be removed entirely.
        if (this_block->ref_count == 1 && last_block->final->type == JUMP
            && strequ(((Jump*) last_block->final)->label, this_block->label)) {

            remove_from_block(last_block, last_block->final);
            for (Statement* state = this_block->first; state;) {
                Statement* this_state = state;
                state = state->next;
                append_to_block(last_block, this_state);
            }

            va_remove(func->basic_blocks, i);
            i -= 1; // Handle the change in size by offsetting i.
        }
    }
}

// Remove needless casting assignments, such as `u8 %0 = 1; u8 %1 = %0;`.
void remove_unused_casts(Function* func) {
    LocalVar* this_local = NULL;
    for (size_t i = 0; this_local = iterate_locals(func, &i); i++) {

        // This only handles direct assignemnt operations.
        if (this_local->origin && this_local->origin->type == OPERATION) {
            Operation* origin_op = (Operation*) this_local->origin;

            if (origin_op->type == ASSIGN && !origin_op->rhs.is_const && this_local->type == func->locals[origin_op->rhs.local_id]->type) {
                for (size_t j = 0; j < va_len(this_local->references); j++) {
                    *this_local->references[j] = origin_op->rhs.local_id;
                }

                remove_from_block(this_local->origin->parent, this_local->origin);

                free_local_var(this_local);
                func->locals[i] = NULL;
            }
        }

    }
}

void fold_constant_operations(Function* func) {
    LocalVar* this_local = NULL;
    for (size_t i = 0; this_local = iterate_locals(func, &i); i++) {
        if (func->locals[i]->origin && func->locals[i]->origin->type == OPERATION) {
            Operation* origin_op = (Operation*) this_local->origin;

            switch (origin_op->type) {
            case NOT: case NEGATE: case COMPLEMENT:
                if (!is_local_const(func->locals[origin_op->lhs]))
                    break;
                Value* const_val = get_local_const(func->locals[origin_op->lhs]);

                switch (origin_op->type) {
                case NOT:
                    origin_op->rhs.const_unsigned = !const_val->const_unsigned;
                    break;
                case NEGATE:
                    origin_op->rhs.is_signed = true;
                    origin_op->rhs.const_signed = -const_val->const_signed;
                    break;
                case COMPLEMENT:
                    origin_op->rhs.const_unsigned = ~const_val->const_unsigned;
                    break;
                }
                origin_op->type = ASSIGN;
                origin_op->rhs.is_const = true;
                origin_op->rhs.is_signed = false;
                break;
            case ASSIGN: case ADDRESS: case DEREFERENCE:
                continue;
            default:
                if (!is_local_const(func->locals[origin_op->lhs]))
                    break;
                if (!origin_op->rhs.is_const && !is_local_const(func->locals[origin_op->rhs.local_id]))
                    break;

                Value* lhs_val = get_local_const(func->locals[origin_op->lhs]);
                Value* rhs_val = origin_op->rhs.is_const ? &origin_op->rhs : get_local_const(func->locals[origin_op->lhs]);
                bool is_signed = lhs_val->is_signed || rhs_val->is_signed;

                // This big, ugly macro helps avoid some big, ugly code.
                // Just imagine 18 of these in a switch statement.
                #define FOLD_BINOP(type, op) \
                    case type: \
                    if (is_signed) \
                        origin_op->rhs.const_signed = (lhs_val->is_signed ? \
                                                       lhs_val->const_signed : \
                                                       lhs_val->const_unsigned) \
                                                    op (rhs_val->is_signed ? \
                                                        rhs_val->const_signed : \
                                                        rhs_val->const_unsigned); \
                    else \
                        origin_op->rhs.const_unsigned = lhs_val->const_unsigned \
                                                        op rhs_val->const_unsigned; \
                    break

                switch (origin_op->type) {
                    FOLD_BINOP(ADD, +);
                    FOLD_BINOP(SUB, -);
                    FOLD_BINOP(MUL, *);
                    FOLD_BINOP(DIV, /);
                    FOLD_BINOP(MOD, %);
                    FOLD_BINOP(B_AND, &&);
                    FOLD_BINOP(B_OR, ||);
                    FOLD_BINOP(B_XOR, ^);
                    FOLD_BINOP(L_AND, &);
                    FOLD_BINOP(L_OR, |);
                    FOLD_BINOP(LSH, <<);
                    FOLD_BINOP(RSH, >>);
                    FOLD_BINOP(LESS, <);
                    FOLD_BINOP(GREATER, >);
                    FOLD_BINOP(LESS_EQU, <=);
                    FOLD_BINOP(GREATER_EQU, >=);
                    FOLD_BINOP(NOT_EQU, !=);
                    FOLD_BINOP(EQU, ==);
                }

                #undef FOLD_BINOP

                origin_op->type = ASSIGN;
                origin_op->rhs.is_const = true;
                origin_op->rhs.is_signed = is_signed;
                break;
            }
        }
    }
}

// Run various optimizations according to the user's options.
void optimize_ir(Declaration** decls) {
    // Remove unused basic blocks.
    for (size_t i = 0; i < va_len(decls); i++) {
        if (decls[i]->is_fn) {
            Function* func = (Function*) decls[i];

            if (remove_unused) {
                remove_unused_blocks(func);
                remove_unused_fallthroughs(func);
                remove_unused_casts(func);
            }
            if (fold_constants) {
                fold_constant_operations(func);
            }
        }
    }
}
