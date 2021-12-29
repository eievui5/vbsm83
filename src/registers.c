#include "registers.h"

// Forward-declare 16-bit regs.

extern const CPUReg bc_reg;
extern const CPUReg de_reg;
extern const CPUReg hl_reg;

const CPUReg a_reg = {"a", 1};
const CPUReg c_reg = {"c", 1, &bc_reg};
const CPUReg b_reg = {"b", 1, &bc_reg};
const CPUReg e_reg = {"e", 1, &de_reg};
const CPUReg d_reg = {"d", 1, &de_reg};
const CPUReg l_reg = {"l", 1, &hl_reg};
const CPUReg h_reg = {"h", 1, &hl_reg};

const CPUReg* bc_children[] = {&b_reg, &c_reg, NULL};
const CPUReg* de_children[] = {&d_reg, &e_reg, NULL};
const CPUReg* hl_children[] = {&h_reg, &l_reg, NULL};

const CPUReg bc_reg = {"bc", 2, NULL, bc_children};
const CPUReg de_reg = {"de", 2, NULL, de_children};
const CPUReg hl_reg = {"hl", 2, NULL, hl_children};

const CPUReg* short_regs[] = {&a_reg, &c_reg, &b_reg, &e_reg, &d_reg, &l_reg, &h_reg, NULL};
const CPUReg* wide_regs[] = {&bc_reg, &de_reg, &hl_reg, NULL};
