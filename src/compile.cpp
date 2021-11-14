#include "parser.hpp"

void compile(UnitContext& unit_context, std::ostream& outfile) {
    for (auto* i : unit_context.statements) {
        i->compile(outfile);
    }
}