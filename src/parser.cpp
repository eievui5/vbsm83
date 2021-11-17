#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "exception.hpp"
#include "parser.hpp"
#include "statements/label.hpp"
#include "statements/return.hpp"
#include "tokenizer.hpp"
#include "types.hpp"

UnitContext::~UnitContext() {
    for (Statement* i : statements) {
        delete i;
    }
}

void LocalVar::print_info(std::ostream& out) {
    out << get_type(variable_type).str << ' ' << identifier << "; ";
    out << "Used " << use_count << " time";
    if (use_count != 1)
        out << 's';
    out<< ". ";
    if (is_returned)
        out << "Is returned. ";
    out << '\n';
}

/* Read a local variable off the token list.
Potentially adds an assignment as well, if the variable is initialized.
*/
void read_local_var(FunctionContext& unit_block, TokenList& token_list) {
    LocalVar* declaration = new LocalVar;

    declaration->variable_type = (VariableType) get_type_from_str(token_list.get_token().string);
    declaration->identifier = token_list.get_token().string;

    token_list.expect("=");
    declaration->value = token_list.get_token().string;
    token_list.expect(";");

    unit_block.local_vars[declaration->identifier] = declaration;
    unit_block.append(declaration);
}

void parse_function_block(Function& function, TokenList& token_list) {
    function.unit_block.return_type = function.variable_type;

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
