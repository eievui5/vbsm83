#include "exception.h"
#include "gb/operations.h"
#include "registers.h"
#include "statements.h"
#include "varray.h"

// Define components of registers.
static CPUReg* a_components[] = {&a_reg, NULL};
static CPUReg* b_components[] = {&b_reg, NULL};
static CPUReg* c_components[] = {&c_reg, NULL};
static CPUReg* d_components[] = {&d_reg, NULL};
static CPUReg* e_components[] = {&e_reg, NULL};
static CPUReg* h_components[] = {&h_reg, NULL};
static CPUReg* l_components[] = {&l_reg, NULL};
static CPUReg* bc_components[] = {&b_reg, &c_reg, NULL};
static CPUReg* de_components[] = {&d_reg, &e_reg, NULL};
static CPUReg* hl_components[] = {&h_reg, &l_reg, NULL};
static CPUReg* bcde_components[] = {&b_reg, &c_reg, &d_reg, &e_reg, NULL};
static CPUReg* dehl_components[] = {&d_reg, &e_reg, &h_reg, &l_reg, NULL};
static CPUReg* hlbc_components[] = {&h_reg, &l_reg, &b_reg, &c_reg, NULL};

// 8-bit registers
CPUReg a_reg = {"a", 1, a_components};
CPUReg b_reg = {"b", 1, b_components};
CPUReg c_reg = {"c", 1, c_components};
CPUReg d_reg = {"d", 1, d_components};
CPUReg e_reg = {"e", 1, e_components};
CPUReg h_reg = {"h", 1, h_components};
CPUReg l_reg = {"l", 1, l_components};

// 16-bit registers
CPUReg bc_reg = {"bc", 2, bc_components};
CPUReg de_reg = {"de", 2, de_components};
CPUReg hl_reg = {"hl", 2, hl_components};

// Note: 24-bit register unions are very much feasible, and would likely be a
// useful addition. Please look into this ASAP.

// 32-bit register unions
CPUReg bcde_reg = {NULL, 4, bcde_components};
CPUReg dehl_reg = {NULL, 4, dehl_components};
CPUReg hlbc_reg = {NULL, 4, hlbc_components};

// Register pools
static CPUReg* regs8[] = {&a_reg, &c_reg, &b_reg, &e_reg, &d_reg, &l_reg, &h_reg, NULL};
static CPUReg* regs16[] = {&bc_reg, &de_reg, &hl_reg, NULL};
static CPUReg* regs32[] = {&bcde_reg, &dehl_reg, &hlbc_reg, NULL};

const uint8_t type_widths[] = {0, 1, 2, 4, 8, 1, 2, 4, 8, 4, 8, 2};

// Recursively check if a register's base components are in use.
static bool is_reg_used(CPUReg* reg) {
    for (size_t i = 0; reg->components[i]; i++) {
        if (reg->components[i]->_in_use)
            return true;
    }
    return false;
}

// Recursively set a register's base components' usage states.
static void set_reg_usage(CPUReg* reg, bool usage) {
    for (size_t i = 0; reg->components[i]; i++) {
        reg->_in_use = usage;
    }
}

// Returns true if reg1 and reg2 have components in common.
static bool match_registers(CPUReg* reg1, CPUReg* reg2) {
    CPUReg** reg1_components = reg1->components;
    CPUReg** reg2_components = reg2->components;

    for (; *reg1_components; reg1_components++) {
        for (; *reg2_components; reg2_components++) {
            if (*reg1_components == * reg2_components)
                return true;
        }
    }

    return false;
}

static void allocate_register(LocalVar* local, CPUReg** reg_pool, size_t when) {
    for (size_t j = 0; reg_pool[j]; j++) {
        if (!is_reg_used(reg_pool[j])) {
            set_reg_usage(reg_pool[j], true);
            va_expand(&local->reg_reallocs, sizeof(RegRealloc));
            RegRealloc* new_reg = &va_last(local->reg_reallocs);
            new_reg->reg = reg_pool[j];
            new_reg->when = when;
            return;
        }
    }

    fatal("Ran out of CPU registers. Stack variables are not yet supported.");
}

// Reallocate a local variable to a different register. This does not set the
// previous register to 'unused', so that the calling code can claim it.
static void spill_local(LocalVar* local, size_t when) {
    CPUReg** reg_pool = NULL;

    // Choose a register pool according to the local's size.
    switch (type_widths[local->type]) {
    case 1: reg_pool = regs8; break;
    case 2: reg_pool = regs16; break;
    case 4: reg_pool = regs32; break;
    }

    if (reg_pool)
        allocate_register(local, reg_pool, when);
    else
        fatal("Ran out of CPU registers. Stack variables are not yet supported.");
}

// Checks if a register is being used by a local variable and relocates that
// variable if it exists.
static void open_register(Function* func, CPUReg* reg, size_t when) {
    if (!is_reg_used(reg))
        return;

    // We know the register is currently used, but not by which variable. Search
    // the function's locals for a variable which is currently using this
    // register.
    LocalVar* this_local = NULL;

    for (size_t i = 0; this_local = iterate_locals(func, &i); i++) {
        if (match_registers(va_last(this_local->reg_reallocs).reg, reg)) {
            // Move the local variable once found.
            warn("Spilling local in register %s.", reg->name);
            spill_local(this_local, when);
            break;
        }
    }
}

void analyze_var_usage(Function* func) {
    for (size_t i = 0; i < func->parameter_count; i++) {
        func->locals[i]->lifetime_start = 0;
    }

    Statement* statement = NULL;
    size_t i = 0;
    size_t block_id = 0;

    while (statement = iterate_statements(func, statement, &i, &block_id)) {
        switch (statement->type) {
        case OPERATION: {
            Operation* op = (Operation*) statement;
            func->locals[op->dest]->lifetime_start = i;
            switch (op->type) {
            default: // binops
                if (!op->rhs.is_const)
                    func->locals[op->rhs.local_id]->lifetime_end = i;
                // fallthrough
            case NOT: case NEGATE: case COMPLEMENT: case ADDRESS: case DEREFERENCE:
                func->locals[op->lhs]->lifetime_end = i;
                break; // unops
            case ASSIGN:
                if (!op->rhs.is_const)
                    func->locals[op->rhs.local_id]->lifetime_end = i;
                break; //assign
            }
        } break;
        case READ: {
        } break;
        case WRITE: {
            Write* write = (Write*) statement;
            func->locals[write->src]->lifetime_end = i;
        } break;
        case RETURN: {
            Return* ret = (Return*) statement;
            if (!ret->val.is_const)
                func->locals[ret->val.local_id]->lifetime_end = i;
        } break;
        }
    }
}

// Iterate through an operation pool until all valid operations have been found.
// Returns NULL when no operations remain. When this occurs, the caller should
// modify the arguments and search again for a valid operation.
const CpuOp* search_for_operation(const CpuOp** operation_pool, size_t* index,
                                  uint8_t dest_width, uint8_t lhs_width,
                                  uint8_t rhs_width, bool is_const) {
    // Continue searching until the end of the array is found (designated by
    // NULL).
    for (; operation_pool[*index]; (*index)++) {
        const CpuOp* cpu_op = operation_pool[*index];

        warn("Attempting to match d:%u l:%u r:%u c:%s == d:%u l:%u r:%u c:%s.",
             dest_width, lhs_width, rhs_width, is_const ? "true" : "false",
             cpu_op->result_width, cpu_op->lhs_width, cpu_op->rhs_width, cpu_op->is_const ? "true" : "false");

        if (cpu_op->result_width == dest_width && cpu_op->lhs_width == lhs_width
         && cpu_op->rhs_width == rhs_width && cpu_op->is_const == is_const)
            return cpu_op;
    }
    return NULL;
}

void select_operation(Function* func, Operation* op, size_t when) {
    // When choosing an operation consider the operation and the width of the
    // operands and result. Attempt to choose an operation which uses the
    // smallest registers possible, and cast operands and results as needed.
    // Additionally, if an binary operation has a constant operand, attempt to
    // choose an operation which supports constants. If one is not available,
    // then insert a move instruction and select a normal operation.

    switch (op->type) {
    case ADD: {
        // Determine initial base properties of an operation.
        size_t i = 0;
        uint8_t dest_width = type_widths[op->var_type];
        uint8_t lhs_width = type_widths[func->locals[op->lhs]->type];
        uint8_t rhs_width;
        if (!op->rhs.is_const) {
            rhs_width = type_widths[func->locals[op->rhs.local_id]->type];
        } else {
            // This could depend on the width needed to store an integer. I'm
            // not sure how I want to handle it yet.
            rhs_width = 0;
        }
        bool is_const = op->rhs.is_const;

        const CpuOp* cpu_op = search_for_operation(add_operations, &i, dest_width, lhs_width, rhs_width, is_const);

        if (cpu_op == NULL)
            fatal("Failed to find operation for variable %%%zu. Variable promotion is not yet supported.",
                  op->dest);

        op->cpu_info.operation = cpu_op;
        // Select registers, cause spills as needed.

        // Claim the operation's required registers.
        for (CPUReg** required_regs = cpu_op->required_regs; *required_regs; required_regs++) {
            set_reg_usage(*required_regs, true);
        }

        // Then for each of these registers, spill any locals that may be using
        // them.
        for (CPUReg** required_regs = cpu_op->required_regs; *required_regs; required_regs++) {
            open_register(func, *required_regs, when);
        }

        // Once this is done the operation's registers have been acounted for
        // and it can later be compiled into assembly code after further
        // processing.
    } break;
    }
}

void assign_registers(Function* func) {
    // Reset usage of all registers.
    for (size_t i = 0; regs8[i]; i++)
        set_reg_usage(regs8[i], false);

    // Assign arguments according to the ABI.
    for (size_t i = 0; i < func->parameter_count; i++) {
        CPUReg** reg_pool = NULL;

        switch (type_widths[func->parameter_types[i]]) {
        case 1: reg_pool = regs8; break; // The ABI skips `a`
        case 2: reg_pool = regs16; break;
        case 4: reg_pool = regs32; break;
        }

        if (reg_pool) {
            allocate_register(func->locals[i], reg_pool, 0);
        } else {
            fatal("No valid CPU registers for paremeter %%%zu in %s", i, func->declaration.identifier);
        }
    }

    Statement* statement = NULL;
    size_t cur_statement = 0;
    size_t block_id = 0;
    while (statement = iterate_statements(func, statement, &cur_statement, &block_id)) {
        //  Begin by allocating any locals needed by this statement.
        LocalVar* this_local = NULL;
        for (size_t i = 0; this_local = iterate_locals(func, &i); i++) {
            // This is a bit hacky, as it was made before spilling was accounted
            // for.
            if (!va_size(this_local->reg_reallocs) && this_local->lifetime_start == cur_statement) {
                CPUReg** reg_pool = NULL;

                // Choose a register pool according to the local's size.
                switch (type_widths[this_local->type]) {
                case 1: reg_pool = regs8; break;
                case 2: reg_pool = regs16; break;
                case 4: reg_pool = regs32; break;
                }

                // If no pool was available, skip straight to the stack.
                if (reg_pool) {
                    allocate_register(this_local, reg_pool, cur_statement);
                } else {
                    fatal("No valid CPU registers for variable %%%zu in %s", i, func->declaration.identifier);
                }
            }

            // When a local variable is no longer used, free its register (if it
            // still has one).
            if (this_local->lifetime_end == cur_statement) {
                if (va_len(this_local->reg_reallocs))
                    set_reg_usage(va_last(this_local->reg_reallocs).reg, false);
            }
        }

        // Choose a register layout for this statement, if one is needed.
        // This is most important for allowing operations to move around
        // registers as needed.
        switch (statement->type) {
        case OPERATION:
            select_operation(func, (Operation*) statement, cur_statement);
            break;
        case READ: break;
        case WRITE: break;
        }
    }

    // Temporarily output some debug info to show how registers were allocated.
    printf("Begin regalloc graph of %s.\n====================\n0  1  2  3  4  5  6  7  8  9\n",
            func->declaration.identifier);
    statement = NULL;
    cur_statement = 0;
    block_id = 0;
    while (statement = iterate_statements(func, statement, &cur_statement, &block_id)) {
        for (size_t i = 0; i < va_len(func->locals); i++) {
            LocalVar* this_local = func->locals[i];

            if (this_local == NULL) {
                fputs("   ", stdout);
            } else if (this_local->lifetime_start <= cur_statement
                       && this_local->lifetime_end >= cur_statement) {
                const char* reg_name = NULL;

                for (size_t i = 0; i < va_len(this_local->reg_reallocs); i++) {
                    if (this_local->reg_reallocs[i].when <= cur_statement) {
                        reg_name = this_local->reg_reallocs[i].reg->name;
                    }
                }

                if (reg_name)
                    printf("%-3s", reg_name);
                else
                    fputs("err", stdout);
            } else {
                fputs(".  ", stdout);
            }
        }
        putchar('\n');
    }
    puts("====================");
}
