//
// Created by crax on 10/13/20.
//

#include "dictionary.h"

void Dictionary::set_named(const std::string &name, CL::RuntimeValue value) {
    _values[name] = std::move(value);
}
