#pragma once

#include <string>

enum class VariableType {
    U8, U16, U32, U64,
    I8, I16, I32, I64,
    F32, F64, PTR, FARPTR,
    VOID,
};

struct BackendType {
	const char* str;
	int size;
};

extern const BackendType TYPES[];

int get_type_from_str(std::string str);
const BackendType& get_type(VariableType type);