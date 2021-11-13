#include <string>

#include "tokenizer.hpp"
#include "types.hpp"

/*
Analyze code blocks, specifically for Single Static Assignment variable
analysis.

Analysis should be valid at any step in the compilation process, by simply
rearranging the token list. This means the order of analysis can effect code
output.
*/

void assign_local(TokenList& token_list, std::string type) {
    info("Assigned %i.", verify_int(std::stoi(token_list.expect_type(
            TokenType::INT,
            "Expected constant integer after assignment (variable assignment is not yet supported).").string),
        (VariableType) get_type_from_str(type)));
    token_list.expect(";");
}

void analyze_variables(TokenList& token_list) {
    int root_index = token_list.index;

    while (token_list.peek_token().string != "}") {
        if (token_list.peek_token().type == TokenType::TYPE) {
            Token& type = token_list.get_token();
            Token& identifier = token_list.get_token();
            if (identifier.type != TokenType::IDENTIFIER)
                fatal("Expected identifier after local variable declaration");
            info("Defined local variable of type %s: %s", type.c_str(),
                 identifier.c_str());

            Token& assignment = token_list.get_token();
            if (assignment.string == "=") {
                assign_local(token_list, type.string);
            } else if (assignment.string == ";") {
                continue;
            } else {
                fatal("Unexpected token \"%s\" after variable declaration.", assignment.c_str());
            }
        } else {
            while (token_list.get_token().string != ";") {}
        }
    }

    token_list.index = root_index;
}