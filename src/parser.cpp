#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "exception.hpp"
#include "parser.hpp"
#include "statements/label.hpp"
#include "tokenizer.hpp"
#include "types.hpp"

Function* function_declaration(TokenList& token_list, StorageClass storage_class) {
    Function* declaration = new Function;
    declaration->storage_class = storage_class;
    Token& return_type = token_list.get_token();

    if (return_type.type != TokenType::TYPE)
        fatal("Expected return type after function declaration, got: %s.", return_type.c_str());

    declaration->variable_type = (VariableType) get_type_from_str(return_type.string);

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
        fatal("%s is not a valid identifier.", token_list.peek_token().c_str());
    declaration->identifier = token_list.peek_token().string;
    token_list.index++;

    token_list.expect("(");
    token_list.expect(")", "Expected closing parenthesis (arguments are not yet supported).");
    token_list.expect("{");

    if (enable_info) {
        std::stringstream infostring;
        declaration->write(infostring);
        info("%s", infostring.str().c_str());
    }

    return declaration;
}

Variable* variable_declaration(TokenList& token_list, StorageClass storage_class) {
    Variable* declaration = new Variable;
    declaration->storage_class = storage_class;

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
        fatal("%s is not a valid identifier.", token_list.peek_token().c_str());
    declaration->identifier = token_list.peek_token().string;
    token_list.index++;

    token_list.expect(";");

    if (enable_info) {
        std::stringstream infostring;
        declaration->write(infostring);
        info("%s", infostring.str().c_str());
    }

    return declaration;
}

Statement* begin_declaration(TokenList& token_list) {
    Token& storage_class = token_list.get_token();
    Token& declaration = token_list.get_token();

    if (storage_class.type != TokenType::STORAGE_CLASS)
        fatal("Unexpected token in declaration: %s", storage_class.c_str());
    if (declaration.type != TokenType::KEYWORD)
        fatal("Unexpected token in declaration: %s", declaration.c_str());

    Statement* statement;

    switch (declaration.keyword) {
    case Keyword::FN:
        statement = function_declaration(token_list, storage_class.storage_class);
        break;
    case Keyword::VAR:
        statement = variable_declaration(token_list, storage_class.storage_class);
        break;
    default:
        fatal(
            "Unexpected keyword in declaration: %s.\n"
            "Declarations must either be a var or fn.",
            declaration.c_str());
    }

    return statement;
}

Label* read_storage_class(TokenList& token_list) {
    // Collect the first key tokens of this label declaration.
    Label* label_declaration = nullptr;
    bool is_function;
    Token& storage_class = token_list.expect_type(TokenType::STORAGE_CLASS);
    Token& declaration_keyword = token_list.expect_type(TokenType::KEYWORD);
    Token& variable_type = token_list.expect_type(TokenType::TYPE);

    // Create either a variable or function depending on the declaration
    // keyword.
    if (declaration_keyword.string == "fn") {
        is_function = true;
        label_declaration = new Function;
    } else if (declaration_keyword.string == "var") {
        is_function = false;
        label_declaration = new Variable;
    } else {
        fatal("Expected \"fn\" or \"var\" after storage class \"%s\"", storage_class.c_str());
    }

    // Determine storage class
    label_declaration->storage_class = (StorageClass) strinstrs(storage_class.string, STORAGE_CLASS);
    if ((int) label_declaration->storage_class == -1)
        fatal("Invalid storage class: %s", storage_class.c_str());

    label_declaration->variable_type = (VariableType) get_type_from_str(variable_type.string);

    // Store traits.
    if (token_list.peek_token().string == "[[") {
        token_list.index++;
        while (token_list.peek_token().string != "]]") {
            label_declaration->trait_list.push_back(token_list.get_token().string);
        }
        token_list.index++;
    }

    label_declaration->identifier = token_list.expect_type(TokenType::IDENTIFIER).string;

    if (is_function) {
        token_list.expect("(");
        while (token_list.get_token().string != ")") {
            warn("Function arguments are not yet suppoted.");
        }
    } else {
        if (token_list.peek_token().string == "=") {
            // See docs/output.md for information on how to implement variable init.
            warn("Variable assignment is not yet supported.");
        } else {
            token_list.expect(";");
        }
    }

    return label_declaration;
}

Statement* read_statement(TokenList& token_list) {
    Statement* statement = nullptr;

    Token& initial_token = token_list.peek_token();

    switch (initial_token.type) {
    case TokenType::STORAGE_CLASS:
        statement = read_storage_class(token_list);
        break;
    default:
        warn("Unhandled token \"%s\".", initial_token.c_str());
        break;
    }

    return statement;
}