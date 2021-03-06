#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

struct Statement;
struct Function;

typedef struct CPUReg {
    // The symbol used to identify the register; how it appears in the output code.
    const char* name;
    // How many bytes wide is the register.
    const size_t size;
    // Most basic forms of the registers that this register contains.
    // In the case of the SM83, this always means the 8-bit registers.
    struct CPUReg** components;

    // Temporarily used during register allocation to mark a register as used.
    bool _in_use;
} CPUReg;

typedef struct RegRealloc {
    size_t when;
    CPUReg* reg;
} RegRealloc;

typedef struct LocalVar {
    uint8_t type;
    // VArray of pointers to any occarance in which this local is referenced.
    uint64_t** references;
    struct Statement* origin;

    // The index of the statement where the variable was first declared.
    size_t lifetime_start;
    // The index of the statement where the variable was last accessed.
    size_t lifetime_end;
    // The index of the last register in `registers` that this variable was placed into.
    size_t active_reg;
    RegRealloc* reg_reallocs;
} LocalVar;

extern CPUReg a_reg;
extern CPUReg c_reg;
extern CPUReg b_reg;
extern CPUReg e_reg;
extern CPUReg d_reg;
extern CPUReg l_reg;
extern CPUReg h_reg;
extern CPUReg bc_reg;
extern CPUReg de_reg;
extern CPUReg hl_reg;

void analyze_var_usage(struct Function* func);
void fprint_var_usage(FILE* out, struct Function* func);
void assign_registers(struct Function* func);
