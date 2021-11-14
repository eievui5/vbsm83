#include <cstdint>
#include <fstream>

#include "exception.hpp"
#include "parser.hpp"
#include "register_allocation.hpp"
#include "statements/label.hpp"
#include "types.hpp"

void put_label(std::ostream& outfile, std::string identifier, StorageClass storage_class) {
    switch (storage_class) {
    case StorageClass::EXPORT:
        outfile << "_" << identifier << "::\n";
        break;
    case StorageClass::STATIC:
        outfile << "__" << identifier << ":\n";
        break;
    case StorageClass::EXTERN:
        error("Cannot generate an external label.");
        break;
    default:
        error("Invalid declaration storage_class: %i", (int) storage_class);
    }
}

void compile_function(std::ostream& outfile, Function* func) {
    outfile << "; ";
    func->write(outfile);
    outfile << '\n';
    put_label(outfile, func->identifier, func->storage_class);
}

void compile_return(std::ostream& outfile, TokenList& token_list, Function& func) {
    // Skip the return token that got us here.
    token_list.index++;

    Token& ret_token = token_list.get_token();

    switch (ret_token.type) {
    case TokenType::INT: {
        warn("OOPS THIS CODE IS DEPRECAYED PLEASE DELETE THANKS.");
    } break;
    case TokenType::IDENTIFIER:
        warn("Identifier returns are not yet supported.");
        break;
    default:
        fatal("Unhandled return token: %s", ret_token.c_str());
        break;
    }

    token_list.expect(";");

    // Finally, return!
    outfile << "\tret\n";
}

void compile_variable(std::ostream& outfile, Variable* var) {
    outfile << "; ";
    var->write(outfile);
    outfile << '\n';
    put_label(outfile, var->identifier, var->storage_class);
    outfile << "\tDS " << get_type(var->variable_type).size << "\n\n";
}

void compile(TokenList& token_list, std::ofstream& outfile) {
    Function* current_function = nullptr;

    while (token_list.remaining()) {
        Token& next_token = token_list.peek_token();

        switch (next_token.type) {
        case TokenType::STORAGE_CLASS: {
            Statement* declaration = begin_declaration(token_list);

            switch (declaration->type) {
            case StatementType::FUNCTION:
                compile_function(outfile, (Function*) declaration);
                if (current_function)
                    delete current_function;
                current_function = (Function*) declaration;
                break;
            case StatementType::VARIABLE:
                compile_variable(outfile, (Variable*) declaration);
                delete declaration;
                break;
            default:
                fatal("Invalid declaration type: %i", (int) declaration->type);
                delete declaration;
                break;
            }
        } break;
        case TokenType::KEYWORD:
            switch (next_token.keyword) {
            case Keyword::RETURN:
                if (current_function) {
                    compile_return(outfile, token_list, *current_function);
                } else {
                    error("Return statement outside of function declaration.");
                }
                break;
            default:
                warn("Unhandled keyword %s", next_token.c_str());
                break;
            }
            break;
        case TokenType::BRACKET:
            switch (next_token.string[0]) {
            case '}':
                if (current_function) {
                    delete current_function;
                    current_function = nullptr;
                    outfile << '\n';
                } else {
                    error("Unexpected }");
                }
                break;
            default:
                warn("Unhandled bracket: %s", next_token.c_str());
                break;
            }
            token_list.index++;
            break;
        default:
            warn("Unhandled token \"%s\"", next_token.c_str());
            token_list.index++;
            break;
        }
    }
}