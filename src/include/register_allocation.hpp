#pragma once

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

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
    std::string identifier;
    // Which CPU register conatins this register? If null, use stack depth.
    const CPUReg* container;
    // Distance from the root of the function. Negatives are used for function parameters.
    int stack_depth;

    LocalVariable(int size);
    LocalVariable(int size, const CPUReg* force_reg);
    ~LocalVariable();

    void set_const(std::ostream& outfile, int value);
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

extern std::vector<LocalVariable*> local_vars;

inline LocalVariable::~LocalVariable() {
    local_vars.erase(std::remove(local_vars.begin(), local_vars.end(), this), local_vars.end());
}