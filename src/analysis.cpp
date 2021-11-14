#include <unordered_map>
#include <vector>

#include "exception.hpp"
#include "parser.hpp"
#include "statements/label.hpp"
#include "statements/local_var.hpp"

bool optimize_unused_assignment = true;

void allocate_locals(FunctionContext& func_context) {
    for (auto* i : func_context.statements) {
        LocalVarDeclaration* declaration = dynamic_cast<LocalVarDeclaration*>(i);
        if (!declaration)
            continue;
        func_context.local_vars[declaration->identifier] =
            new LocalVariable(func_context.local_vars, get_type(declaration->variable_type).size);
    }
}

void analyze_unused_assignment(UnitContext& unit_context) {
    // A list of assignments which have not yet been used.
    std::unordered_map<std::string, int> assignment_map;
    // A vector of assignment indexes which where not used.
    std::vector<int> unused_assignments;

    for (long unsigned int i = 0; i < unit_context.statements.size(); i++) {
        LocalVarAssignment* assignment = dynamic_cast<LocalVarAssignment*>(unit_context.statements[i]);
        if (!assignment)
            continue;
        if (assignment_map.contains(assignment->identifier)) {
            info("Removing unused assignment of %s.", assignment->identifier.c_str());
            warn("Unused assignement optimizations currently break SSA and are only a POC.");
            assignment_map.erase(assignment->identifier);
            unused_assignments.push_back(i);
        }
        assignment_map[assignment->identifier] = i;
    }

    // Now remove any unused assignments we found.
    for (long unsigned int i = unused_assignments.size(); i > 0; i--) {
        unit_context.statements.erase(unit_context.statements.begin() + i);
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

        if (optimize_unused_assignment)
            analyze_unused_assignment(func->unit_block);
    }
}