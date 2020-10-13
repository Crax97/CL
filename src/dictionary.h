//
// Created by crax on 10/13/20.
//
#pragma once


#include <unordered_map>
#include "commons.hpp"
#include "value.hpp"

class Dictionary {
private:
    std::unordered_map<CL::String, CL::RuntimeValue> _values;
public:
    void set_named(const std::string& name, CL::RuntimeValue value);
};
