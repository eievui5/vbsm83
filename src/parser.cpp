#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "exception.hpp"
#include "parser.hpp"
#include "tokenizer.hpp"

Function function_declaration(TokenList& token_list, Keyword locality) {
    Function declaration;
    Token& return_type = token_list.get_token();

    if (return_type.type != TokenType::TYPE)
        fatal("Expected return type after function declaration, got: %s", return_type.string);

    declaration.return_type = (VariableType) strinstrs(return_type.string.c_str(), TYPES);

    // Check for and parse the trait list.
    if (token_list.peek_token().string == "[[") {
        token_list.index++;
        while (1) {
            Token& next_trait = token_list.get_token();
            if (next_trait.string == "]]")
                break;
            declaration.trait_list.push_back(next_trait.string);
        }
    }

    return declaration;
}

Statement begin_declaration(TokenList& token_list) {
    Token& locality = token_list.get_token();
    Token& declaration = token_list.get_token();

    if (locality.type != TokenType::KEYWORD)
        fatal("Unexpected token in declaration: %s", locality.string.c_str());
    if (declaration.type != TokenType::KEYWORD)
        fatal("Unexpected token in declaration: %s", declaration.string.c_str());

    switch (locality.keyword) {
    case Keyword::EXPORT: case Keyword::EXTERN: case Keyword::STATIC:
        break;
    default:
        fatal("Unexpected keyword in declaration: %s.\n"
              "Declarations should start with extern, export, or static.",
              locality.string.c_str());
    }

    Statement statement;

    switch (declaration.keyword) {
    case Keyword::FN:
        statement = function_declaration(token_list, locality.keyword);
        break;
    case Keyword::VAR:
        break;
    default:
        fatal("Unexpected keyword in declaration: %s.\n"
              "Declarations must either be a var or fn.",
              locality.string.c_str());
    }

    return statement;
}