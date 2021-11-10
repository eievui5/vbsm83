#pragma once

#include <fstream>

#include "parser.hpp"

void compile_function(std::ostream& outfile, Function* func);
void compile_variable(std::ostream& outfile, Variable* var);