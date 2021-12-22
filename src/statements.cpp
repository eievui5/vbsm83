#include <vector>

#include "statements.hpp"
#include "tokenizer.hpp"

using std::vector;

static uptr<VariableDeclaration> parse_declaration(TokenCluster& cluster) {
    uptr<VariableDeclaration> fn;

    if (cluster[1]->string == "fn")
        fn = std::make_unique<FunctionDeclaration>();
    else if (cluster[1]->string == "var") {
        fn = std::make_unique<VariableDeclaration>();
    } else {
        error("Unexpected token \"%s\" after storage class. Expected either \"fn\" or \"var\"",
            cluster[1]->c_str());
    }

    fn->tokens = &cluster;
    fn->storage_class = (StorageClass) strinstrs(cluster[0]->string, STORAGE_CLASS);
    if ((int) fn->storage_class == -1)
        error("Invalid storage class \"%s\". Expected \"static\", \"export\", or \"extern\".",
            cluster[0]->c_str());

    fn->type = (VariableType) get_type_from_str(cluster[2]->string);
    if ((int) fn->type == -1)
        error("Invalid type \"%s\".", cluster[2]->c_str());

    int i = 3;
    if (cluster[i]->string == "[[") {
        i++;
        while (cluster[i]->string != "]]") { i++; }
        i++;
    }
    fn->identifier = cluster[i++]->string;

    if (cluster[1]->string == "fn") {
        if (cluster[i++]->string != "(")
            error("Expected parenthesis t begin argument list.");
        while (cluster[i]->string != ")") {
            warn("Function args are unhandled.");
            i++;
        }
        i++;
    }

    return fn;
}

StatementBlock parse_declarations(TokenList& tokens) {
    StatementBlock declarations;

    while (tokens.remaining()) {
        TokenCluster& cluster = *tokens.get_cluster();

        switch (cluster[0]->type) {
        case TokenType::STORAGE_CLASS:
            declarations.push_back(parse_declaration(cluster));
            break;
        default:
            error("Invalid token in root declaraton: \"%s\".", cluster[0]->c_str());
            break;
        }
    }

    return declarations;
}

StatementBlock parse_statements(TokenList& token_list) {
    StatementBlock outlist;
    for (auto& i : token_list.clusters) {
        if (i.size() == 0)
            continue;
        switch (i[0]->type) {
        case TokenType::STORAGE_CLASS:
            error("Unexpected %s in function body", i[0]->c_str());
            break;
        case TokenType::TYPE: {
            if (i[2]->string == "=") {
                // Handle direct assignment.
                if (i.size() == 4) {
                    uptr<AssignStatement> assign = std::make_unique<AssignStatement>();
                    assign->tokens = &i;
                    assign->dest = i[1]->string;
                    assign->source = i[3]->string;
                    outlist.push_back(std::move(assign));
                } else if (i.size() == 5) {
                    warn("Unops are currently unhandled.");
                } else if (i.size() == 6) {
                    uptr<BinOpStatement> op = std::make_unique<BinOpStatement>();
                    op->tokens = &i;
                    op->dest_type = (VariableType) get_type_from_str(i[0]->string);
                    if ((int) op->dest_type == -1) {
                        error("Unrecognized type \"%s\".", i[0]->c_str());
                        // In the event of an error, pick a common value and try to continue for now.
                        op->dest_type = VariableType::I16;
                    }
                    op->dest = i[1]->string;
                    op->lhs = i[3]->string;
                    op->type = (BinOpType) strinstrs(i[4]->string, OPERATORS);
                    op->rhs = i[5]->string;

                    outlist.push_back(std::move(op));
                }
            } else {
                error("Malformed or unhandled statment beginning with type.");
            }
        } break;
        case TokenType::KEYWORD: {
            if (i[0]->string == "return") {
                uptr<ReturnStatement> ret = std::make_unique<ReturnStatement>();
                ret->tokens = &i;
                ret->source = i[1]->string;

                outlist.push_back(std::move(ret));
            }
        } break;
        default: {
            warn("Unhandled token in statement parsing: %s.", i[0]->c_str());
        } break;
        }
    }
    return outlist;
}
