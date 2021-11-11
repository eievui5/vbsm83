#include <cstdint>
#include <fstream>

#include "exception.hpp"
#include "parser.hpp"
#include "register_allocation.hpp"

template <typename T> T _verify(int num, VariableType type) {
    if ((T) num != num) {
        warn("%i does not fit within a %s. Truncating value to %i.", num, get_type(type).str, (T) num);
    }
    return (T) num;
}

int verify_int(int num, VariableType type) {
    switch (type) {
    case VariableType::U8:
        return _verify<uint8_t>(num, type);
    case VariableType::U16:
        return _verify<uint16_t>(num, type);
    case VariableType::I8:
        return _verify<int8_t>(num, type);
    case VariableType::I16:
        return _verify<int16_t>(num, type);

    default:
        warn("Unhandled variable type: %s", get_type(type).str);
        return 0;
    }
}

void put_label(std::ostream& outfile, std::string identifier, DeclLocal locality) {
    switch (locality) {
    case DeclLocal::EXPORT:
        outfile << "_" << identifier << "::\n";
        break;
    case DeclLocal::STATIC:
        outfile << "__" << identifier << ":\n";
        break;
    case DeclLocal::EXTERN:
        error("Cannot generate an external label.");
        break;
    default:
        error("Invalid declaration locality: %i", (int) locality);
    }
}

void compile_function(std::ostream& outfile, Function* func) {
    outfile << "; " << *func << '\n';
    put_label(outfile, func->identifier, func->locality);
}

void compile_return(std::ostream& outfile, TokenList& token_list, Function& func) {
    // Skip the return token that got us here.
    token_list.index++;

    Token& ret_token = token_list.get_token();

    switch (ret_token.type) {
    case TokenType::INT: {
        LocalVariable return_variable{
            get_type(func.return_type).size, get_type(func.return_type).size == 1 ? &c_reg : &bc_reg};
        return_variable.set_const(outfile, verify_int(std::stoi(ret_token.string), func.return_type));
    } break;
    case TokenType::IDENTIFIER:
        warn("Identifier returns are not yet supported.");
        break;
    default:
        fatal("Unhandled return token: %s", ret_token.string.c_str());
        break;
    }

    token_list.expect(";");

    // Finally, return!
    outfile << "\tret\n";
}

void compile_variable(std::ostream& outfile, Variable* var) {
    outfile << "; " << *var << '\n';
    put_label(outfile, var->identifier, var->locality);
    outfile << "\tDS " << get_type(var->variable_type).size << "\n\n";
}

void compile(TokenList& token_list, std::ofstream& outfile) {
    Function* current_function = nullptr;

    while (token_list.remaining()) {
        Token& next_token = token_list.peek_token();

        switch (next_token.type) {
        case TokenType::LOCALITY: {
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
                warn("Unhandled keyword %s", next_token.string.c_str());
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
                warn("Unhandled bracket: %s", next_token.string.c_str());
                break;
            }
            token_list.index++;
            break;
        default:
            warn("Unhandled token \"%s\"", next_token.string.c_str());
            token_list.index++;
            break;
        }
    }
}