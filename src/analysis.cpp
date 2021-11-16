#include <unordered_map>
#include <vector>

#include "exception.hpp"
#include "parser.hpp"
#include "register_allocation.hpp"
#include "statements/label.hpp"

void allocate_locals(FunctionContext& func_context) {
    for (auto* i : func_context.statements) {
        LocalVar* local_var = dynamic_cast<LocalVar*>(i);
        if (!local_var)
            continue;
        local_var->container = allocate_register(func_context.local_vars, get_type(local_var->variable_type).size);
        func_context.local_vars[local_var->identifier] = local_var;
    }
}

void analyze_unit(UnitContext& unit_context) {
    info("Beginning unit analysis.");

    // Loop through every statement looking for a function.
    for (auto* i : unit_context.statements) {
        Function* func = dynamic_cast<Function*>(i);
        if (!func)
            continue;

        // When a function is found, run any applicable analysis on it.
        info("Analyzing function \"%s\".", func->identifier.c_str());

        // Locals MUST be allocated to compile a function.
        allocate_locals(func->unit_block);
    }
}
