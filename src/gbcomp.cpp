#include <cstdint>
#include <fstream>

#include "exception.hpp"
#include "parser.hpp"

template <typename T>
T _verify(int num, VariableType type) {
    if ((T) num != num) {
        warn("%i does not fit within a %s. Truncating value to %i.",
                num, TYPES[(int) type], (T) num);
    }
    return (T) num;
}

int verify_constant(int constant, VariableType type) {
    switch (type) {
    case VariableType::U8:
        return _verify<uint8_t>(constant, type);
    case VariableType::U16:
        return _verify<uint16_t>(constant, type);
    case VariableType::I8:
        return _verify<int8_t>(constant, type);
    case VariableType::I16:
        return _verify<int16_t>(constant, type);

    default:
        warn("Unhandled variable type: %s", TYPES[(int) type]);
        return 0;
    }
}

void compile_function(std::ostream& outfile, Function* func) {
    outfile << "; " << *func << '\n';
    switch (func->locality) {
    case DeclLocal::EXPORT:
        outfile << "_" << func->identifier << "::\n";
        break;
    case DeclLocal::STATIC:
        outfile << "__" << func->identifier << ":\n";
        break;
    default:
        break;
    }
}

void compile_return(std::ostream& outfile, TokenList& token_list, Function& func) {
    // Skip the return token that got us here.
    token_list.index++;

    Token& ret_token = token_list.get_token();

    switch (ret_token.type) {
    case TokenType::INT:
        switch (func.return_type) {
        case VariableType::U8: case VariableType::I8:
            outfile << "\tld c, " << verify_constant(std::stoi(ret_token.string), func.return_type) << '\n';
        case VariableType::U16: case VariableType::I16:
            outfile << "\tld bc, " << verify_constant(std::stoi(ret_token.string), func.return_type) << '\n';

        }
        break;
    case TokenType::IDENTIFIER:
        warn("Identifier returns are not yet supported.");
        break;
    default:
        fatal("Unhandled return token: %s", ret_token.string.c_str());
        break;
    }

    // Finally, return!
    outfile << "\tret\n";
}

void compile_variable(std::ostream& outfile, Variable* var) {
    outfile << "; " << *var << '\n';
    warn("Variables are not yet supported.");
    switch (var->locality) {
    case DeclLocal::EXPORT:
        outfile << "_" << var->identifier << "::\n";
        break;
    case DeclLocal::STATIC:
        outfile << "__" << var->identifier << ":\n";
        break;
    default:
        break;
    }
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
        default:
            warn("Unhandled token type from \"%s\"", next_token.string.c_str());
            token_list.index++;
            break;
        }
    }
}