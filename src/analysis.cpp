#include <unordered_map>
#include <vector>

#include "exception.hpp"
#include "parser.hpp"
#include "register_allocation.hpp"
#include "statements/label.hpp"
#include "statements/return.hpp"

void analyze_usage(FunctionContext& func_context) {
    for (auto& i : func_context.statements) {
        LocalVar* local_var = dynamic_cast<LocalVar*>(i);
        if (local_var) {
            if (determine_token_type(local_var->value) == TokenType::IDENTIFIER) {
                func_context.local_vars[local_var->value]->use_count++;
            }
            continue;
        }
        ReturnStatement* ret_statement = dynamic_cast<ReturnStatement*>(i);
        if (ret_statement) {
            if (determine_token_type(ret_statement->value) == TokenType::IDENTIFIER) {
                func_context.local_vars[ret_statement->value]->is_returned = true;
                func_context.local_vars[ret_statement->value]->use_count++;
            }
            continue;
        }
    }
}

void allocate_locals(FunctionContext& func_context) {
    for (auto& i : func_context.local_vars) {
        LocalVar* local_var = dynamic_cast<LocalVar*>(i.second);
        if (!local_var)
            continue;
        local_var->container = allocate_register(func_context.local_vars, get_type(local_var->variable_type).size);
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

        // Run analysis.
        analyze_usage(func->unit_block);

        if (info()) {
            std::cout << "Local variable count: " << func->unit_block.local_vars.size() << ".\n";
            for (auto& i : func->unit_block.local_vars) {
                info();
                i.second->print_info(std::cout);
            }
        }

        // Locals MUST be allocated to compile a function.
        allocate_locals(func->unit_block);
    }
}
