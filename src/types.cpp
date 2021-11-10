#include <string>

#include "types.hpp"

const BackendType TYPES[] = {
    {"u8", 1}, {"u16", 2}, {"u32", 4}, {"u64", 8},
    {"i8", 1}, {"i16", 2}, {"i32", 4}, {"i64", 8},
    {"f32", 4}, {"f64", 8},
    {"p", 2}, {"farp", 4}, {"void", 0},
    {nullptr}
};

int get_type_from_str(std::string str) {
    for (int i = 0; TYPES[i].str != nullptr; i++) {
        if (str == TYPES[i].str)
            return i;
    }
    return -1;
}

const BackendType& get_type(VariableType type) {
	return TYPES[(int) type];
}