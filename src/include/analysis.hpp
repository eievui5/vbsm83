#pragma once

#include "parser.hpp"

extern bool optimize_unused_assignment;

void analyze_unused_assignment(UnitContext& unit_context);
void analyze_unit(UnitContext& unit_context);