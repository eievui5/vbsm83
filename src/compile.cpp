#include "exception.hpp"
#include "parser.hpp"

void compile(UnitContext& unit_context, std::ostream& outfile) {
    info("Beginning compilation.");
    for (auto* i : unit_context.statements) {
        i->define(outfile);
    }
}