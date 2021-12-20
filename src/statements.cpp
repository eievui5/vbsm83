#include <vector>

#include "statements.hpp"
#include "tokenizer.hpp"

using std::vector;

vector<vector<Token*>> collect_statements(TokenList& token_list) {
    // Reset token_list position.
    vector<vector<Token*>> out_matrix = {{}};
    token_list.seek(0);
    while (token_list.remaining()) {
        Token& next_token = token_list.get_token();
        char first_char = next_token.string[0];
        if (first_char == ';' || first_char == '{' || first_char == '}') {
            out_matrix.push_back({});
            continue;
        }
        out_matrix.back().push_back(&next_token);
    }
    return out_matrix;
}

vector<stmnt::Statement> parse_statements(TokenList& token_list) {
    vector<stmnt::Statement> outlist;
    auto statement_tokens = collect_statements(token_list);
    for (auto& i : statement_tokens) {
        if (i.size() == 0)
            continue;
        switch (i[0]->type) {
        case TokenType::STORAGE_CLASS: {
            if (i[1]->string == "var") {
                stmnt::Variable var;
                var.tokens = i;
                var.storage_class = (StorageClass) strinstrs(i[0]->string, STORAGE_CLASS);
                var.type = (VariableType) get_type_from_str(i[2]->string);
                if ((int) var.type == -1) {
                    error("Unrecognized type \"%s\".", i[2]->c_str());
                    // In the event of an error, pick a common value and try to continue for now.
                    var.type = VariableType::I16;
                }
                if (i[3]->string == "[[") {
                    int j = 4;
                    for (; i[j]->string != "]]"; j++) {
                        var.traits.push_back(i[j]->string);
                    }
                    j++;
                    var.identifier = i[j]->string;
                } else {
                    var.identifier = i[3]->string;
                }
                outlist.push_back(var);
            } else if (i[1]->string == "fn") {
                stmnt::Variable fn;
                fn.tokens = i;
                fn.storage_class = (StorageClass) strinstrs(i[0]->string, STORAGE_CLASS);
                fn.type = (VariableType) get_type_from_str(i[2]->string);
                if ((int) fn.type == -1) {
                    error("Unrecognized type \"%s\".", i[2]->c_str());
                    // In the event of an error, pick a common value and try to continue for now.
                    fn.type = VariableType::I16;
                }
                int j = 3;
                if (i[3]->string == "[[") {
                    j++;
                    while (i[j]->string != "]]")
                        fn.traits.push_back(i[j++]->string);
                    j++;
                }
                fn.identifier = i[j++]->string;
                if (i[j++]->string != "(")
                    error("Expected parenthesis t begin argument list.");
                while (i[j]->string != ")") {
                    warn("Function args are unhandled.");
                    j++;
                }

                outlist.push_back(fn);
            }
        } break;
        case TokenType::TYPE: {
            if (i[2]->string == "=") {
                // Handle direct assignment.
                if (i.size() == 4) {
                    stmnt::Assignment assign;
                    assign.tokens = i;
                    assign.dest = i[1]->string;
                    assign.source = i[3]->string;
                    outlist.push_back(assign);
                } else if (i.size() == 5) {
                    warn("Unops are currently unhandled.");
                } else if (i.size() == 6) {
                    stmnt::BinOp op;
                    op.tokens = i;
                    op.dest_type = (VariableType) get_type_from_str(i[0]->string);
                    if ((int) op.dest_type == -1) {
                        error("Unrecognized type \"%s\".", i[0]->c_str());
                        // In the event of an error, pick a common value and try to continue for now.
                        op.dest_type = VariableType::I16;
                    }
                    op.dest = i[1]->string;
                    op.lhs = i[3]->string;
                    op.type = (BinOpType) strinstrs(i[4]->string, BIN_OPS);
                    op.rhs = i[5]->string;

                    outlist.push_back(op);
                }
            } else {
                error("Malformed or unhandled statment beginning with type.");
            }
        } break;
        case TokenType::KEYWORD: {
            if (i[0]->string == "return") {
                stmnt::Return ret;
                ret.tokens = i;
                ret.source = i[1]->string;

                outlist.push_back(ret);
            }
        } break;
        default: {
            warn("Unhandled token in statement parsing: %s.", i[0]->c_str());
        } break;
        }
    }
    return outlist;
}
