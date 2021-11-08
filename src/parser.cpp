#include <memory>
#include <string>
#include <vector>

class Statement {
public:
    std::string string;
};

Statement* collect_statement(std::vector& token_list) {

    return new Statement;
}