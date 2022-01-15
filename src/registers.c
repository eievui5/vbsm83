#include "exception.h"
#include "registers.h"
#include "statements.h"
#include "varray.h"

// Define children of large registers.
CPUReg* bc_children[] = {&b_reg, &c_reg, NULL};
CPUReg* de_children[] = {&d_reg, &e_reg, NULL};
CPUReg* hl_children[] = {&h_reg, &l_reg, NULL};

CPUReg* bcde_children[] = {&bc_reg, &de_reg, NULL};
CPUReg* dehl_children[] = {&de_reg, &hl_reg, NULL};
CPUReg* hlbc_children[] = {&hl_reg, &bc_reg, NULL};

// 8-bit registers
CPUReg a_reg = {"a", 1};
CPUReg c_reg = {"c", 1, &bc_reg};
CPUReg b_reg = {"b", 1, &bc_reg};
CPUReg e_reg = {"e", 1, &de_reg};
CPUReg d_reg = {"d", 1, &de_reg};
CPUReg l_reg = {"l", 1, &hl_reg};
CPUReg h_reg = {"h", 1, &hl_reg};

// 16-bit registers
CPUReg bc_reg = {"bc", 2, NULL, bc_children};
CPUReg de_reg = {"de", 2, NULL, de_children};
CPUReg hl_reg = {"hl", 2, NULL, hl_children};

// Note: 24-bit register unions are very much feasible, and would likely be a
// useful addition. Please look into this ASAP.

// 32-bit register unions
CPUReg bcde_reg = {NULL, 4, NULL, bcde_children};
CPUReg dehl_reg = {NULL, 4, NULL, dehl_children};
CPUReg hlbc_reg = {NULL, 4, NULL, hlbc_children};

// Register pools
CPUReg* regs8[] = {&a_reg, &c_reg, &b_reg, &e_reg, &d_reg, &l_reg, &h_reg, NULL};
CPUReg* regs16[] = {&bc_reg, &de_reg, &hl_reg, NULL};
CPUReg* regs32[] = {&bcde_reg, &dehl_reg, &hlbc_reg, NULL};

const uint8_t type_widths[] = {0, 1, 2, 4, 8, 1, 2, 4, 8, 4, 8, 2};

// Recursively check if a register's base components are in use.
static bool is_reg_used(CPUReg* reg) {
    if (reg->children) {
        for (size_t i = 0; reg->children[i]; i++) {
            if (is_reg_used(reg->children[i]))
                return true;
        }
        return false;
    } else {
        return reg->_in_use;
    }
}

// Recursively set a register's base components' usage states.
static void set_reg_usage(CPUReg* reg, bool usage) {
    if (reg->children) {
        for (size_t i = 0; reg->children[i]; i++) {
            set_reg_usage(reg->children[i], usage);
        }
    } else {
        reg->_in_use = usage;
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

void assign_registers(Function* func) {
    // Reset usage of all registers.
    for (size_t i = 0; regs8[i]; i++)
        set_reg_usage(regs8[i], false);

    // Assign arguments according to the ABI.
    for (size_t i = 0; i < func->parameter_count; i++) {
        CPUReg** reg_pool = NULL;

        switch (type_widths[func->parameter_types[i]]) {
        case 1: reg_pool = regs8 + 1; break; // The ABI skips `a`
        case 2: reg_pool = regs16; break;
        case 4: reg_pool = regs32; break;
        }

        if (reg_pool) {
            size_t j;
            for (j = 0; reg_pool[j]; j++) {
                if (!is_reg_used(reg_pool[j])) {
                    set_reg_usage(reg_pool[j], true);
                    va_expand(&func->locals[i]->reg_reallocs, sizeof(RegRealloc));
                    RegRealloc* new_reg = &va_last(func->locals[i]->reg_reallocs);
                    new_reg->reg = reg_pool[j];
                    new_reg->when = 0;
                    break;
                }
            }
            if (reg_pool[j] == NULL)
                fatal("Ran out of CPU registers for paremeter %%%zu in %s", i, func->declaration.identifier);
        } else {
            fatal("No valid CPU registers for paremeter %%%zu in %s", i, func->declaration.identifier);
        }
    }

    Statement* statement = NULL;
    size_t cur_statement = 0;
    size_t block_id = 0;
    while (statement = iterate_statements(func, statement, &cur_statement, &block_id)) {
        LocalVar* this_local = NULL;
        for (size_t i = 0; this_local = iterate_locals(func, &i); i++) {
            if (!va_size(this_local->reg_reallocs) && this_local->lifetime_start == cur_statement) {
                CPUReg** reg_pool = NULL;

                switch (type_widths[this_local->type]) {
                case 1: reg_pool = regs8; break;
                case 2: reg_pool = regs16; break;
                case 4: reg_pool = regs32; break;
                }

                if (reg_pool) {
                    size_t j;
                    for (j = 0; reg_pool[j]; j++) {
                        if (!is_reg_used(reg_pool[j])) {
                            set_reg_usage(reg_pool[j], true);
                            va_expand(&this_local->reg_reallocs, sizeof(RegRealloc));
                            RegRealloc* new_reg = &va_last(func->locals[i]->reg_reallocs);
                            new_reg->reg = reg_pool[j];
                            new_reg->when = cur_statement;
                            break;
                        }
                    }
                    if (reg_pool[j] == NULL)
                        fatal("Ran out of CPU registers for variable %%%zu in %s", i, func->declaration.identifier);
                } else {
                    fatal("No valid CPU registers for variable %%%zu in %s", i, func->declaration.identifier);
                }
            }
            if (this_local->lifetime_end == cur_statement) {
                if (va_len(this_local->reg_reallocs))
                    set_reg_usage(va_last(this_local->reg_reallocs).reg, false);
            }
        }
    }

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
                RegRealloc* this_reg = &this_local->reg_reallocs[this_local->active_reg];
                if (va_size(this_local->reg_reallocs) && this_reg->reg)
                    printf("%-3s", this_reg->reg->name);
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
