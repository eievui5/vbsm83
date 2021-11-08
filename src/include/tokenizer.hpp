#include <string>

enum class TokenType {
    NONE,
    COMMENT,
    OPERATOR,
    TYPE,
    IDENTIFIER,
    TRAIT,
    BRACKET,
    COMMA,
    SEMICOLON,
    INT,
    FLOAT,
    KEYWORD,
};

enum class Keyword {
    EXTERN, EXPORT, STATIC,
    FN, VAR,
    RETURN,
};

class Token {
public:
    TokenType type = TokenType::NONE;
    Keyword keyword;
    std::string string;

    void determine_type();
};

extern const char COMMENT[];
extern const char BRACKETS[];
extern const char* KEYWORDS[];
extern const char NUMBERS[];
extern const char* OPERATORS[];
extern const char SINGLES[];
extern const char SYMBOLS[];
extern const char* TYPES[];
extern const char WHITESPACE[];

Token* read_token(std::ifstream& input);