#pragma once

#include <string>

#include "parser.hpp"

class ReturnStatement : public FuncStatement {
  public:
    std::string value;

    void compile(std::ostream& outfile, FunctionContext& context);
};