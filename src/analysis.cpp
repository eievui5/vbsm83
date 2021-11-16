#include <unordered_map>
#include <vector>

#include "exception.hpp"
#include "parser.hpp"
#include "statements/label.hpp"
#include "statements/local_var.hpp"

void analyze_unit(UnitContext& unit_context) {
    info("Beginning unit analysis.");

    // Loop through every statement looking for a function.
    for (auto* i : unit_context.statements) {
        Function* func = dynamic_cast<Function*>(i);
        if (!func)
            continue;

        // When a function is found, run any applicable analysis on it.
        info("Analyzing function \"%s\".", func->identifier.c_str());

    }
}