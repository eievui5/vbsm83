#include "exception.h"
#include "registers.h"
#include "statements.h"
#include "varray.h"

// Forward-declare 16-bit regs.

extern CPUReg bc_reg;
extern CPUReg de_reg;
extern CPUReg hl_reg;

CPUReg a_reg = {"a", 1};
CPUReg c_reg = {"c", 1, &bc_reg};
CPUReg b_reg = {"b", 1, &bc_reg};
CPUReg e_reg = {"e", 1, &de_reg};
CPUReg d_reg = {"d", 1, &de_reg};
CPUReg l_reg = {"l", 1, &hl_reg};
CPUReg h_reg = {"h", 1, &hl_reg};

const CPUReg* bc_children[] = {&b_reg, &c_reg, NULL};
const CPUReg* de_children[] = {&d_reg, &e_reg, NULL};
const CPUReg* hl_children[] = {&h_reg, &l_reg, NULL};

CPUReg bc_reg = {"bc", 2, NULL, bc_children};
CPUReg de_reg = {"de", 2, NULL, de_children};
CPUReg hl_reg = {"hl", 2, NULL, hl_children};

CPUReg* short_regs[] = {&a_reg, &c_reg, &b_reg, &e_reg, &d_reg, &l_reg, &h_reg, NULL};
CPUReg* wide_regs[] = {&bc_reg, &de_reg, &hl_reg, NULL};

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

void fprint_var_usage(FILE* out, Function* func) {
    fprintf(out, "Begin variable usage graph of %s.\n====================\n0 1 2 3 4 5 6 7 8 9\n",
            func->declaration.identifier);
    Statement* statement = NULL;
    size_t cur_statement = 0;
    size_t block_id = 0;
    while (statement = iterate_statements(func, statement, &cur_statement, &block_id)) {
        for (size_t i = 0; i < va_len(func->locals); i++) {
            LocalVar* this_local = func->locals[i];

            if (this_local == NULL) {
                fputc(' ', out);
            } else if (this_local->lifetime_start == cur_statement
                || this_local->lifetime_end == cur_statement) {
                fputc('#', out);
            } else if (this_local->lifetime_start < cur_statement
                       && this_local->lifetime_end > cur_statement) {
                fputc('|', out);
            } else {
                fputc('.', out);
            }
            fputc(' ', out);
        }
        fprint_statement(out, statement);
    }
    fputs("====================\n", out);
}

void assign_registers(Function* func) {
    Statement* statement = NULL;
    size_t cur_statement = 0;
    size_t block_id = 0;
    while (statement = iterate_statements(func, statement, &cur_statement, &block_id)) {
        LocalVar* this_local = NULL;
        for (size_t i = 0; this_local = iterate_locals(func, &i); i++) {
            if (this_local->lifetime_start == cur_statement) {
                CPUReg** reg_pool = NULL;

                switch (type_widths[this_local->type]) {
                case 1: reg_pool = short_regs; break;
                case 2: reg_pool = wide_regs; break;
                }

                if (reg_pool) {
                    for (size_t j = 0; reg_pool[j]; j++) {
                        if (is_reg_used(reg_pool[j])) {
                            set_reg_usage(reg_pool[j], true);
                            this_local->reg = reg_pool[j];
                        }
                    }
                } else {
                    fatal("Ran out of CPU registers for variable %%%zu in %s", i, func->declaration.identifier);
                }
            } else if (this_local->lifetime_end == cur_statement) {
                if (this_local->reg) {
                    set_reg_usage(this_local->reg, false);
                    this_local->reg = NULL;
                } else {
                    error("Freeing local variable without register assignment.");
                }
            }
        }
    }
}
