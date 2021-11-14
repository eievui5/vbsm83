#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "exception.hpp"
#include "parser.hpp"
#include "statements/label.hpp"
#include "statements/local_var.hpp"
#include "statements/return.hpp"
#include "tokenizer.hpp"
#include "types.hpp"

UnitContext::~UnitContext() {
    for (Statement* i : statements) {
        delete i;
    }
}

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

/* Read a local variable off the token list.
Potentially adds an assignment as well, if the variable is initialized.
*/
void read_local_var(UnitContext& unit_block, TokenList& token_list) {
    LocalVarDeclaration* declaration = new LocalVarDeclaration;

    declaration->variable_type = (VariableType) get_type_from_str(token_list.get_token().string);
    declaration->identifier = token_list.get_token().string;

    unit_block.append(declaration);

    if (token_list.peek_token().string == "=") {
        token_list.index++;
        LocalVarAssignment* assignment = new LocalVarAssignment;
        assignment->identifier = declaration->identifier;
        assignment->value = token_list.get_token().string;
        token_list.expect(";");
        unit_block.append(assignment);
    } else {
        token_list.expect(";");
    }
}

void parse_function_block(Function& function, TokenList& token_list) {
    while (token_list.peek_token().string != "}") {
        switch (token_list.peek_token().type) {
        case TokenType::KEYWORD:
            if (token_list.peek_token().string == "return") {
                ReturnStatement* return_statement = new ReturnStatement;
                token_list.index++;
                return_statement->value = token_list.get_token().string;
                token_list.expect(";");
                function.unit_block.append(return_statement);
            } else {
                warn("Unhandled keyword \"%s\".", token_list.peek_token().c_str());
            }
            break;
        case TokenType::IDENTIFIER:
            // Identifiers are vague, so we need to do an additional check on them.
            if (token_list.peek_token(1).string == "=") {
                // This is quite short so I don't need a function for this case.
                LocalVarAssignment* assignment = new LocalVarAssignment;
                assignment->identifier = token_list.get_token().string;
                token_list.expect("=");
                assignment->value = token_list.get_token().string;
                token_list.expect(";");
                function.unit_block.append(assignment);
            } else {
                warn("Unhandled identifier \"%s\".", token_list.peek_token().c_str());
            }
            break;
        case TokenType::TYPE:
            read_local_var(function.unit_block, token_list);
            break;
        default:
            warn("Unhandled token \"%s\".", token_list.peek_token().c_str());
            token_list.index++;
            break;
        }
    }
    token_list.index++;
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
        if (label_declaration->storage_class != StorageClass::EXTERN) {
            token_list.expect("{");
            parse_function_block(*dynamic_cast<Function*>(label_declaration), token_list);
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
        token_list.index++;
        break;
    }

    return statement;
}

void parse_token_list(UnitContext& context, TokenList& token_list) {
    while (token_list.remaining()) {
        Statement* debug_statement = read_statement(token_list);
        if (debug_statement) {
            context.statements.push_back(debug_statement);
        }
    }
}