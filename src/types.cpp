#include <string>

#include "exception.hpp"
#include "types.hpp"

const BackendType TYPES[] = {
    {   "u8", 1},
    {  "u16", 2},
    {  "u32", 4},
    {  "u64", 8},
    {   "i8", 1},
    {  "i16", 2},
    {  "i32", 4},
    {  "i64", 8},
    {  "f32", 4},
    {  "f64", 8},
    {    "p", 2},
    { "farp", 4},
    { "void", 0},
    {nullptr  }
};

template <typename T> T _verify(int num, VariableType type) {
    if ((T) num != num) {
        warn("%i does not fit within a %s. Truncating value to %i.", num, get_type(type).str, (T) num);
    }
    return (T) num;
}

int verify_int(int num, VariableType type) {
    switch (type) {
    case VariableType::U8:
        return _verify<uint8_t>(num, type);
    case VariableType::U16:
        return _verify<uint16_t>(num, type);
    case VariableType::I8:
        return _verify<int8_t>(num, type);
    case VariableType::I16:
        return _verify<int16_t>(num, type);

    default:
        warn("Unhandled variable type: %s", get_type(type).str);
        return 0;
    }
}

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