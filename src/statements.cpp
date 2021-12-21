#include <vector>

#include "statements.hpp"
#include "tokenizer.hpp"

using std::vector;

vector<VariableDeclaration> parse_declarations(TokenList& tokens) {

}

StatementBlock parse_statements(TokenList& token_list) {
    StatementBlock outlist;
    for (auto& i : token_list.clusters) {
        if (i.size() == 0)
            continue;
        switch (i[0].type) {
        case TokenType::STORAGE_CLASS: {
            if (i[1].string == "var") {
                VariableDeclaration var;
                var.tokens = &i;
                var.storage_class = (StorageClass) strinstrs(i[0].string, STORAGE_CLASS);
                var.type = (VariableType) get_type_from_str(i[2].string);
                if ((int) var.type == -1) {
                    error("Unrecognized type \"%s\".", i[2].c_str());
                    // In the event of an error, pick a common value and try to continue for now.
                    var.type = VariableType::I16;
                }
                if (i[3].string == "[[") {
                    int j = 4;
                    for (; i[j].string != "]]"; j++) {
                        var.traits.push_back(i[j].string);
                    }
                    j++;
                    var.identifier = i[j].string;
                } else {
                    var.identifier = i[3].string;
                }
                outlist.push_back(var);
            } else if (i[1].string == "fn") {
                FunctionDeclaration fn;
                fn.tokens = &i;
                fn.storage_class = (StorageClass) strinstrs(i[0].string, STORAGE_CLASS);
                fn.type = (VariableType) get_type_from_str(i[2].string);
                if ((int) fn.type == -1) {
                    error("Unrecognized type \"%s\".", i[2].c_str());
                    // In the event of an error, pick a common value and try to continue for now.
                    fn.type = VariableType::I16;
                }
                int j = 3;
                if (i[3].string == "[[") {
                    j++;
                    while (i[j].string != "]]")
                        fn.traits.push_back(i[j++].string);
                    j++;
                }
                fn.identifier = i[j++].string;
                if (i[j++].string != "(")
                    error("Expected parenthesis t begin argument list.");
                while (i[j].string != ")") {
                    warn("Function args are unhandled.");
                    j++;
                }

                outlist.push_back(fn);
            }
        } break;
        case TokenType::TYPE: {
            if (i[2].string == "=") {
                // Handle direct assignment.
                if (i.size() == 4) {
                    AssignStatement assign;
                    assign.tokens = &i;
                    assign.dest = i[1].string;
                    assign.source = i[3].string;
                    outlist.push_back(assign);
                } else if (i.size() == 5) {
                    warn("Unops are currently unhandled.");
                } else if (i.size() == 6) {
                    BinOpStatement op;
                    op.tokens = &i;
                    op.dest_type = (VariableType) get_type_from_str(i[0].string);
                    if ((int) op.dest_type == -1) {
                        error("Unrecognized type \"%s\".", i[0].c_str());
                        // In the event of an error, pick a common value and try to continue for now.
                        op.dest_type = VariableType::I16;
                    }
                    op.dest = i[1].string;
                    op.lhs = i[3].string;
                    op.type = (BinOpType) strinstrs(i[4].string, BIN_OPS);
                    op.rhs = i[5].string;

                    outlist.push_back(op);
                }
            } else {
                error("Malformed or unhandled statment beginning with type.");
            }
        } break;
        case TokenType::KEYWORD: {
            if (i[0].string == "return") {
                ReturnStatement ret;
                ret.tokens = &i;
                ret.source = i[1].string;

                outlist.push_back(ret);
            }
        } break;
        default: {
            warn("Unhandled token in statement parsing: %s.", i[0].c_str());
        } break;
        }
    }
    return outlist;
}
