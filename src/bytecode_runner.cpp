//
// Created by gsoli on 17/12/2020.
//

#include "bytecode_runner.hpp"

#include <utility>
#include "environment.hpp"

namespace CL {
    Opcode BytecodeRunner::decode(uint8_t code) {
        return byte_to_opcode(code);
    }

    void BytecodeRunner::execute(Opcode op) {
        switch (op) {

            case Opcode::Nop:
                break;
            case Opcode::Load_Literal: {
                int constant_index = fetch32();
                push(constants[constant_index]);
            }
                break;
            case Opcode::Load: {
                uint16_t name_index = fetch16();
                std::string name = symbol_table->get_name(name_index);;
                push(current_stack_frame().environment->get(name));
            }
                break;
            case Opcode::Store: {
                uint16_t name_index = fetch16();
                std::string name = symbol_table->get_name(name_index);
                auto value = pop();
                current_stack_frame().environment->assign(name, value);
                push(value);
                }
                break;
            case Opcode::Set: {
                auto value = pop();
                auto what = pop();
                auto who = pop();
                who.set_property(what, value);
                push(value);
            }
                break;
            case Opcode::Get:{
                auto what = pop();
                auto who = pop();
                push(who.get_property(what));
                }
                break;
            case Opcode::List:
                make_list();
                break;
            case Opcode::Dict:
                make_dict();
                break;
            case Opcode::Neg:
                peek().negate();
                break;
            case Opcode::Add:
                push(pop() + pop());
                break;
            case Opcode::Sub: {
                auto left = pop();
                auto right = pop();
                push(left - right);
            }
                break;
            case Opcode::Mul: {
                auto left = pop();
                auto right = pop();
                push(left * right);
            }
                break;
            case Opcode::Div: {
                auto left = pop();
                auto right = pop();
                push(left / right);
            }
                break;
            case Opcode::Mod: {
                auto left = pop();
                auto right = pop();
                push(left.modulo(right));
            }
                break;
            case Opcode::Pow: {
                auto left = pop();
                auto right = pop();
                push(left.to_power_of(right));
            }
                break;
            case Opcode::Less: {
                auto left = pop();
                auto right = pop();
                push(left < right);
            }
                break;
            case Opcode::Less_Eq: {
                auto left = pop();
                auto right = pop();
                push(left <= right);
            }
                break;
            case Opcode::Greater: {
                auto left = pop();
                auto right = pop();
                push(left > right);
            }
                break;
            case Opcode::Greater_Eq: {
                auto left = pop();
                auto right = pop();
                push(left >= right);
            }
                break;
            case Opcode::Eq: {
                auto left = pop();
                auto right = pop();
                push(left == right);
            }
                break;
            case Opcode::Neq: {
                auto left = pop();
                auto right = pop();
                push(left != right);
            }
                break;
            case Opcode::True:
                push(pop().is_truthy());
                break;
            case Opcode::Push_Frame:
                push_frame(StackFrame {
                    std::make_shared<StackedEnvironment>(current_stack_frame().environment),
                            current_stack_frame().code,
                            current_stack_frame().program_counter
                });
                break;
            case Opcode::Pop_Frame:
                pop_frame();
                break;
            case Opcode::Jump_True: {
                uint32_t addr = fetch32();
                if (pop().is_truthy()) {
                    current_stack_frame().program_counter = addr;
                }
            }
                break;
            case Opcode::Jump_False: {
                uint32_t addr = fetch32();
                if (!pop().is_truthy()) {
                    current_stack_frame().program_counter = addr;
                }
            }
                break;
            case Opcode::Jump:
                current_stack_frame().program_counter = fetch32();
                break;
            case Opcode::Call:
                call_function();
                break;
            case Opcode::Module:
                break;
            case Opcode::Return: {
                auto return_value = pop();
                current_stack_frame().program_counter = current_stack_frame().code.size();
                push(return_value);
            }
                break;
            case Opcode::Break:
            case Opcode::Continue:
                current_stack_frame().program_counter = current_stack_frame().code.size();
                break;
            case Opcode::Get_Iter:
                break;
            case Opcode::Iter_Has_Next:
                push(peek().get_named("__has_next").as<CallablePtr>()->call().value().is_truthy());
                break;
            case Opcode::Get_Iter_Next:
                push(peek().get_named("__next").as<CallablePtr>()->call().value());
                break;
        }
    }

    void BytecodeRunner::call_function() {
        uint8_t call_arity = fetch8();
        auto callable = pop().as<CallablePtr>();
        std::vector<RuntimeValue> argument_values;
        argument_values.reserve(call_arity);
        for (int i = 0; i < call_arity; i++) {
            argument_values.push_back(pop());
        }
        auto as_bytecode_fn = std::dynamic_pointer_cast<BytecodeFunction>(callable);
        // TODO Kinda hacky
        // If it's not a bytecode function and there is a result
        if (as_bytecode_fn == nullptr) {
            auto result = callable->call(argument_values);
            if (result.has_value()) push(result.value());
        } else {
            auto call_env = std::make_shared<StackedEnvironment>(current_stack_frame().environment);
            for (int i = 0; i < as_bytecode_fn->argument_names.size(); i++) {
                auto argument_name = as_bytecode_fn->argument_names[i];
                call_env->bind(argument_name, argument_values[as_bytecode_fn->argument_names.size() - i - 1]);
            }
            push_frame(StackFrame{
                    call_env,
                    as_bytecode_fn->bytecode,
                    0
            });
        }
    }

    void BytecodeRunner::loop() {
        while (has_frames()) {
            while (current_stack_frame().program_counter < current_stack_frame().code.size()) {
                uint8_t instruction = fetch8();
                Opcode op = decode(instruction);
                execute(op);
            }
            pop_frame();
        }
    }

    uint8_t BytecodeRunner::fetch8() {
        return current_stack_frame().code[current_stack_frame().program_counter++];
    }

    uint16_t BytecodeRunner::fetch16() {
        return static_cast<uint16_t>(fetch8() << 8) | fetch8();
    }

    uint32_t BytecodeRunner::fetch32() {
        return static_cast<uint32_t>(fetch8() << 24) | static_cast<uint32_t>(fetch8() << 16) |
               static_cast<uint32_t>(fetch8() << 8) | fetch8();
    }

    void BytecodeRunner::push_frame(StackFrame frame) {
        execution_frames.push(std::move(frame));
    }

    StackFrame BytecodeRunner::pop_frame() {
        auto frame = execution_frames.top();
        execution_frames.pop();
        return frame;
    }

    StackFrame &BytecodeRunner::current_stack_frame() {
        return execution_frames.top();
    }

    std::optional<RuntimeValue> BytecodeRunner::run() {
        loop();
        return program_result;
    }

    BytecodeRunner::BytecodeRunner(std::vector<uint8_t> main_chunk, SymbolTablePtr in_symbol_table,
                                   RuntimeEnvPtr env)
                                   : symbol_table(std::move(in_symbol_table)) {
        push_frame(StackFrame {
                std::move(env),
                std::move(main_chunk),
        });
    }

    void BytecodeRunner::make_list() {
        int elements = fetch32();
        std::shared_ptr<List> list = std::make_shared<List>();
        for (int i = 0; i < elements; i ++) {
            list->append(pop());
        }
        push(RuntimeValue(list));
    }

    void BytecodeRunner::make_dict() {
        int elements = fetch32();
        std::shared_ptr<Dictionary> dict = std::make_shared<Dictionary>();
        for (int i = 0; i < elements; i ++) {
            auto value = pop();
            auto key = pop();
            dict->set(key, value);
        }
        push(RuntimeValue(dict));
    }

    std::optional<RuntimeValue> BytecodeFunction::call(const Args &args) {
        if(runner.expired()) throw CLException("This function's bytecode runner hasn't been set!");
        auto runner_pointer = runner.lock();
        runner_pointer->push_frame(StackFrame {
            std::make_shared<StackedEnvironment>(runner_pointer->current_stack_frame().environment),
            bytecode
        });
        return std::optional<RuntimeValue>();
    }

    BytecodeFunction::BytecodeFunction(std::shared_ptr<BytecodeRunner> in_runner,
                                                       std::vector<uint8_t> in_bytecode,
                                                       std::vector<std::string> in_argument_names,
                                                       bool in_is_variadic)
    : runner(std::move(in_runner)), bytecode(std::move(in_bytecode)), argument_names(std::move(in_argument_names)), is_variadic(in_is_variadic) {}
}