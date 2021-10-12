#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exception.h"
#include "parser.h"

typedef struct Function {
    Context* context;
    char* name;
    int return_type;
} Function;

void function_context(Context* context, FILE* output) {
    Function function = {context};

    // Get the function's return type.
    function.return_type = strinstrs(seek_token(context, -3)->string, TYPES);
    if (function.return_type < 0) {
        error("Invalid return type %s", seek_token(context, -3)->string);
    }

    // Collect traits.
    while (remaining_tokens(context) > 0) {
        Token* next = next_token(context);

        switch (next->type) {
            case TK_IDENTIFIER:
                if (strcmp(next->string, "noninline") == 0) {
                    info(output, "; noninline");
                } else if (strcmp(next->string, "pure") == 0) {
                    info(output, "; pure");
                }
                break;
            case TK_BRACKET:
                // Only "]]" should appear in a trait list (to
                // end the list).
                if (expect_token(context, "]")) {
                    goto collect_name;
                } else {
                    error("Unhandled bracket \"%s\" in function declaration.", next->string);
                }
                break;
            default:
                error("Unhandled token \"%s\" in function declaration.", next->string);
                break;
        }
    }

    collect_name: {
        Token* next = next_token(context);

        if (next->type == TK_IDENTIFIER) {
            function.name = next->string;
            fprintf(output, "SECTION \"_%s function\" ROM0\n_%s:\n", function.name, function.name);
        } else {
            error("Expected identifier name for function!");
        }
    }

    // A parenthesis must follow to begin arg list.
    if (!expect_token(context, "(")) {
        error("Expected ( to begin argument list!");
    }

    // Collect parameter list.
    while (remaining_tokens(context) > 0) {
        Token* next = next_token(context);

        switch (next->type) {
            case TK_BRACKET:
                if (next->string[0] == ')') {
                    if (expect_token(context, "{")) {
                        goto handle_block;
                    } else {
                        error("Expected opening brace after \"%s\" argument list.", function.name);
                    }
                } else {
                    error("Unhandled bracket \"%s\" in \"%s\" argument list.", next->string, function.name);
                }
                break;
            case TK_TYPE: {
                Token* identifier = next_token(context);

                if (identifier->type != TK_IDENTIFIER) {
                    error("Expected identifier after argument type.");
                } else {
                    printf("Found argument of type %s named \"%s\"\n", next->string, identifier->string);
                }

                if (peek_token(context)->type == TK_COMMA) {
                    context->cur_token++;
                }
            } break;
            default:
                error("Unhandled token %s.", next->string);
                break;


        }
    }

    handle_block: {
        printf("Compiling function \"%s\" with return type %s.\n", function.name, TYPES[function.return_type]);

        while (remaining_tokens(context) > 0) {
            Token* next = next_token(context);

            switch (next->type) {
                case TK_BRACKET:
                    if (next->string[0] == '}') {
                        goto exit;
                    }
                    break;
                case TK_COMMENT:
                    fprintf(output, "\t; %s\n", next->string);
                    break;
                case TK_KEYWORD:
                    printf("Identified keyword: %s\n", next->string);
                    break;
                case TK_TYPE: {
                    Token* identifier = next_token(context);

                    if (!identifier->type == TK_IDENTIFIER) {
                        error("Expected identifier after type %s", next->string);
                    }
                } break;
                default:
                    error("Unhandled token %s.", next->string);
                    break;
            }
        }
    }
    exit:
    puts("Finished function declaration.\n");
    fprintf(output, "\n");
    return;
}

void compile_context(Context* context, FILE* output) {
    // Folder Root.
    // Search for tokens to begin context modes.

    while (remaining_tokens(context) > 0) {
        Token* next = next_token(context);
        switch (next->type) {
            case TK_COMMENT:
                fprintf(output, "; %s\n", next->string);
                break;
            case TK_TYPE:
                // Expect two tokens to begin function trait list.
                if (expect_token(context, "[") && expect_token(context, "[")) {
                    function_context(context, output);
                } else {
                    error("Unhandled type %s.", next->string);
                }
                break;
            case TK_NONE:
                error("Unknown token %s.", next->string);
                break;
            default:
                error("Unhandled token %s.", next->string);
                break;
        }
    }
}