//
// Created by gsoli on 01/12/2020.
//

#pragma once

#include "nodes.hpp"
#include "stack_based_evaluator.hpp"



namespace CL {
    using OpcodeValue = uint8_t;
    using Value = uint32_t;
    class StackFrame {
        std::string name;
        std::vector<OpcodeValue> Opcodes;
        void add_opcode(OpcodeValue op) { }
        void add_opcode(OpcodeValue op, Value value) { }
    };

    std::vector<std::string> names;

    class VMASTEvaluator :
            public StackMachine<StackFrame>, public Evaluator {
    private:
    };
}