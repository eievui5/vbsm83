#pragma once

#include <algorithm>
#include <fstream>
#include <string>
#include <unordered_map>

class FunctionContext;

struct CPUReg {
    // The symbol used to identify the register; how it appears in the output code.
    const std::string name;
    // How many bytes wide is the register.
    const int size;
    // Register which owns this register, if any.
    const CPUReg* parent;
    // Registers that this register contains.
    const CPUReg** children;
};

class LocalVariable {
  public:
    // Which register is this variable stored in?
    const CPUReg* container;
    int size;

    void assign(std::ostream& outfile, FunctionContext& context, std::string identifier, std::string value);
};

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

void allocate_locals(FunctionContext& func_context);