#include "label.hpp"

// Move these to `label.cpp`
void Function::write(std::ostream& os) {
    os << STORAGE_CLASS[(int) storage_class] << ' ' << get_type(variable_type).str << " [[ ";
    for (auto& i : trait_list) {
        os << i << ", ";
    }
    os << "]] " << identifier << "();";
}

void Variable::write(std::ostream& os) {
    os << STORAGE_CLASS[(int) storage_class] << ' ' << get_type(variable_type).str << " [[ ";
    for (auto& i : trait_list) {
        os << i << ", ";
    }
    os << "]] " << identifier << ";";
}