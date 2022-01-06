#pragma once

#include <stdlib.h>

struct Statement;

typedef struct CPUReg {
    // The symbol used to identify the register; how it appears in the output code.
    const char* name;
    // How many bytes wide is the register.
    const size_t size;
    // Register which owns this register, if any.
    const struct CPUReg* parent;
    // Registers that this register contains.
    const struct CPUReg** children;
} CPUReg;

typedef struct LocalVar {
    uint8_t type;
    // VArray of pointers to any occarance in which this local is referenced.
    size_t** references;
    struct Statement* origin;
} LocalVar;

extern const CPUReg a_reg;
extern const CPUReg c_reg;
extern const CPUReg b_reg;
extern const CPUReg e_reg;
extern const CPUReg d_reg;
extern const CPUReg l_reg;
extern const CPUReg h_reg;
extern const CPUReg bc_reg;
extern const CPUReg de_reg;
extern const CPUReg hl_reg;
