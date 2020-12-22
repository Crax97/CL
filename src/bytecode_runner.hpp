//
// Created by gsoli on 17/12/2020.
//

#pragma once

#include "commons.hpp"
#include "value.hpp"
#include "stack_based_evaluator.hpp"
#include "vm_ast_evaluator.h"

namespace CL {
    struct StackFrame {
        RuntimeEnvPtr environment;
        std::vector<uint8_t> code;
        int program_counter = 0;
        std::optional<RuntimeValue> return_value;
    };
#if 0
    struct ExternalCallFrame
            : public StackFrame {
        explicit ExternalCallFrame(std::optional<RuntimeValue> result) {
            return_value = std::move(result);
            environment = nullptr;
            program_counter = 69420;
            code = {};
        }
    };
#endif
    class BytecodeFunction
            : public Callable {
        friend class BytecodeRunner;
    private:
        std::weak_ptr<BytecodeRunner> runner;
        std::vector<uint8_t> bytecode;
        std::vector<std::string> argument_names;
        bool is_variadic;

    public:
        explicit BytecodeFunction(std::shared_ptr<BytecodeRunner> in_runner,
                                  std::vector<uint8_t> in_bytecode,
                                  std::vector<std::string> in_argument_names,
                                  bool in_is_variadic = false);
        std::optional<RuntimeValue> call(const Args &args) override;
        uint8_t arity() override { return argument_names.size(); }
        [[nodiscard]]
        std::string to_string() const noexcept override {
            return "Function";
        }
    };
    class BytecodeRunner
            :   public StackMachine<RuntimeValue> {
        friend class BytecodeFunction;
    private:
        SymbolTablePtr symbol_table;
        std::stack<StackFrame> execution_frames;
        std::optional<RuntimeValue> program_result;

        std::vector<RuntimeValue> constants;
        static Opcode decode(uint8_t code);
        void execute(Opcode op);
        void loop();
        uint8_t fetch8();
        uint16_t fetch16();
        uint32_t fetch32();

        void make_list();
        void make_dict();

    protected:
        [[nodiscard]] bool has_frames() const {
            return !execution_frames.empty();
        }
        void push_frame(StackFrame frame);
        StackFrame pop_frame();
        StackFrame& current_stack_frame();

    public:
        explicit  BytecodeRunner(std::vector<uint8_t> main_chunk,
                                 SymbolTablePtr in_symbol_table,
                                 RuntimeEnvPtr env);

        std::optional<RuntimeValue> run();
        void set_constants(std::vector<RuntimeValue> in_constants) {
            constants = std::move(in_constants);
        }

        [[nodiscard]] bool stack_has_value() const { return !m_stack.empty(); }
        [[nodiscard]] RuntimeValue get_last_stack_value() { return pop(); }
        void call_function();
    };
}
