#pragma once

#include <string>

#include "parser.hpp"

class ReturnStatement : public Statement {
  public:
    std::string value;
};