#include <vector>

#include "exception.hpp"
#include "register_allocation.hpp"

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

const CPUReg* bc_children[] = {&b_reg, &c_reg, nullptr};
const CPUReg* de_children[] = {&d_reg, &e_reg, nullptr};
const CPUReg* hl_children[] = {&h_reg, &l_reg, nullptr};

const CPUReg bc_reg = {"bc", 2, nullptr, bc_children};
const CPUReg de_reg = {"de", 2, nullptr, de_children};
const CPUReg hl_reg = {"hl", 2, nullptr, hl_children};

const CPUReg* short_regs[] = {&a_reg, &c_reg, &b_reg, &e_reg, &d_reg, &l_reg, &h_reg, nullptr};
const CPUReg* wide_regs[] = {&bc_reg, &de_reg, &hl_reg, nullptr};

LocalVariable::LocalVariable(std::unordered_map<std::string, LocalVariable*>& local_vars, int size) : size(size) {
    const CPUReg** reg_pool = nullptr;

    // Determine which register pool to use for this size.
    if (size == 1) {
        reg_pool = short_regs;
    } else if (size == 2) { // or size == 4) {
        reg_pool = wide_regs;
    }

    // Try selecting a register pool if we can use one.
    if (reg_pool) {
        while (*reg_pool != nullptr) {
            // Check if the register we want is used.
            for (auto& i : local_vars) {
                // If any matches are found, check the next register.
                const CPUReg* target_reg = *reg_pool;
                if (i.second->container == *reg_pool or i.second->container == target_reg->parent)
                    goto next_reg;
                if (target_reg->children != nullptr) {
                    for (int j = 0; target_reg->children[j] != nullptr; j++) {
                        if (target_reg->children[j] == i.second->container)
                            goto next_reg;
                    }
                }
            }

            // If we make it here, a valid register was found. Give it to the
            // local var and exit.
            container = *reg_pool;
            info("Selected register %s for %i-byte value.", container->name.c_str(), size);
            return;
        next_reg:
            reg_pool++;
        }
    }

    // If we're here, then we need to allocate space on the stack for ourselves.
    // However, I don't wanna write this logic right now. Let's just error out
    // for now.
    fatal("Ran out of registers for size of %i. Stack variables are not yet supported.", size);
    return;
}