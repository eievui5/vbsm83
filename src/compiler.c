#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exception.h"
#include "parser.h"

typedef enum Registers {
    RG_NONE, RG_A, RG_B, RG_C, RG_D, RG_E, RG_H, RG_L
} Registers;

typedef struct TempVar {
    // Where is the temp var currently stored?
    Registers reg;
    int value;
} TempVar;

size_t temp_count = 0;
TempVar* temp_vars = NULL;

/* Push a TempVar to the temp_vars.
 * Allocates a new entry and copies the temp var into it.
*/
void temp_push(TempVar temp) {
    temp_count++;
    temp_vars = realloc(temp_vars, temp_count * sizeof(TempVar));
    temp_vars[temp_count - 1] = temp;
}

/* Delete a given entry from the temp_vars
*/
void temp_delete(size_t i) {
    memmove(&temp_vars[i], &temp_vars[i + 1], (temp_count - i) * sizeof(TempVar));
    temp_count--;
}

void function_context(Context* context, FILE* output) {
    char* function_name = NULL;

    puts("Found a function declaration.");

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
            printf("Function name is \"%s\"\n", next->string);
            function_name = next->string;
            fprintf(output, "_%s:\n", function_name);
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
                        error("Expected opening brace after \"%s\" argument list.", function_name);
                    }
                } else {
                    error("Unhandled bracket \"%s\" in \"%s\" argument list.", next->string, function_name);
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
                    skip_token(context);
                }
            } break;
            default:
                error("Unhandled token %s.", next->string);
                break;


        }
    }

    handle_block: {
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
                case TK_TYPE: {
                    if (!expect_token(context, "%")) {
                        error("Expected %% after type %s", next->string);
                    }
                    
                    Token* identifier = next_token(context);
                    
                    //size_t tempvar_i = strtoul(identifier->string);
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