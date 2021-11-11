#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "exception.hpp"
#include "parser.hpp"
#include "tokenizer.hpp"
#include "types.hpp"

std::ostream& operator<<(std::ostream& os, Function function) {
    os << LOCALITY[(int) function.locality] << ' ' << get_type(function.return_type).str << " [[ ";
    for (auto& i : function.trait_list) {
        os << i << ", ";
    }
    os << "]] " << function.identifier << "();";
    return os;
}

std::ostream& operator<<(std::ostream& os, Variable variable) {
    os << LOCALITY[(int) variable.locality] << ' ' << get_type(variable.variable_type).str << " [[ ";
    for (auto& i : variable.trait_list) {
        os << i << ", ";
    }
    os << "]] " << variable.identifier << ";";
    return os;
}

Function* function_declaration(TokenList& token_list, DeclLocal locality) {
    Function* declaration = new Function;
    declaration->locality = locality;
    Token& return_type = token_list.get_token();

    if (return_type.type != TokenType::TYPE)
        fatal("Expected return type after function declaration, got: %s.", return_type.string.c_str());

    declaration->return_type = (VariableType) get_type_from_str(return_type.string);

    // Check for and parse the trait list.
    if (token_list.peek_token().string == "[[") {
        token_list.index++;
        while (1) {
            Token& next_trait = token_list.get_token();
            if (next_trait.string == "]]")
                break;
            declaration->trait_list.push_back(next_trait.string);
        }
    }

    if (token_list.peek_token().type != TokenType::IDENTIFIER)
        fatal("%s is not a valid identifier.", token_list.peek_token().string.c_str());
    declaration->identifier = token_list.peek_token().string;
    token_list.index++;

    token_list.expect("(");
    token_list.expect(")", "Expected closing parenthesis (arguments are not yet supported).");
    token_list.expect("{");

    if (enable_info) {
        std::stringstream infostring;
        infostring << *declaration;
        info("%s", infostring.str().c_str());
    }

    return declaration;
}

Variable* variable_declaration(TokenList& token_list, DeclLocal locality) {
    Variable* declaration = new Variable;
    declaration->locality = locality;

    declaration->variable_type = (VariableType) get_type_from_str(token_list.get_token().string);

    // Check for and parse the trait list.
    if (token_list.peek_token().string == "[[") {
        token_list.index++;
        while (1) {
            Token& next_trait = token_list.get_token();
            if (next_trait.string == "]]")
                break;
            declaration->trait_list.push_back(next_trait.string);
        }
    }

    if (token_list.peek_token().type != TokenType::IDENTIFIER)
        fatal("%s is not a valid identifier.", token_list.peek_token().string.c_str());
    declaration->identifier = token_list.peek_token().string;
    token_list.index++;

    token_list.expect(";");

    if (enable_info) {
        std::stringstream infostring;
        infostring << *declaration;
        info("%s", infostring.str().c_str());
    }

    return declaration;
}

Statement* begin_declaration(TokenList& token_list) {
    Token& locality = token_list.get_token();
    Token& declaration = token_list.get_token();

    if (locality.type != TokenType::LOCALITY)
        fatal("Unexpected token in declaration: %s", locality.string.c_str());
    if (declaration.type != TokenType::KEYWORD)
        fatal("Unexpected token in declaration: %s", declaration.string.c_str());

    Statement* statement;

    switch (declaration.keyword) {
    case Keyword::FN:
        statement = function_declaration(token_list, locality.locality);
        break;
    case Keyword::VAR:
        statement = variable_declaration(token_list, locality.locality);
        break;
    default:
        fatal(
            "Unexpected keyword in declaration: %s.\n"
            "Declarations must either be a var or fn.",
            declaration.string.c_str());
    }

    return statement;
}