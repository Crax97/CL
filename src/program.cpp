//
// Created by gsoli on 17/12/2020.
//

#include "program.h"
#include "vm_ast_evaluator.h"
#include "bytecode_runner.hpp"
#include "environment.hpp"

#include <fstream>
#include <ostream>
#include <filesystem>
namespace CL {
    struct LiteralTypeVisitor {
        LiteralType operator()(const Number) { return LiteralType::Number; }
        LiteralType operator()(const String&) { return LiteralType::String; }
    };

class FileException : public CLException {
public:
    explicit FileException(const std::filesystem::path& path)
        : CLException(std::string("Could not open file ") + path.generic_string()) {}
};

template<typename T>
void write(std::ostream& stream, T element) {
    char *el_ptr = reinterpret_cast<char *>(&element);
    int size = sizeof(T);
    for (int i = size - 1; i >= 0; i --) {
        stream.write(&el_ptr[i], 1);
    }
}

void write_name(std::ostream& stream, String s) {
    stream.write(s.c_str(), s.size() + 1);
}

void write_number(std::ostream& stream, Number n) {
    char type_byte = static_cast<char>(LiteralType::Number);
    stream.write(&type_byte, 1);
    write(stream, n);
}
void write_string(std::ostream& stream, String s) {
    char type_byte = static_cast<char>(LiteralType::String);
    stream.write(&type_byte, 1);
    stream.write(s.c_str(), s.size() + 1);
}
void write_function(std::ofstream& output_file_stream, const FunctionInfo& info) {
    write<uint16_t>(output_file_stream, info.name_index);
    write<uint8_t>(output_file_stream, info.arg_count);
    for (int i = 0; i < info.arg_count; i ++) {
        uint16_t arg_name_index = info.args[i];
        write<uint16_t>(output_file_stream, arg_name_index);
    }
    write<uint64_t>(output_file_stream, info.code_size);
    for(uint8_t code : info.bytecode) {
        write<uint8_t>(output_file_stream, code);
    }
}

void CompiledProgram::write_to_file(const std::string &file_path) {
    auto path = std::filesystem::path(file_path);
    auto output_file_stream = std::ofstream(path, std::ios_base::out);
    if (!(path.has_filename() && output_file_stream.is_open())) { throw FileException(file_path); }

    ProgramHeader header;
    header.timestamp = std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
    header.name_count = symbol_table->names.size();
    header.function_count = symbol_table->functions.size();
    header.literals_count = symbol_table->literals.size();

    write_header(output_file_stream, header);
    write_names(output_file_stream);
    write_literals(output_file_stream);
    write_functions(output_file_stream);
    write_bytecode(output_file_stream);
}

    void CompiledProgram::write_literals(std::basic_ofstream<char> &output_file_stream) {
        for (auto& literal : symbol_table->literals) {
            LiteralType literal_type = std::visit(LiteralTypeVisitor{}, literal);
            switch(literal_type) {
                case LiteralType::Number:
                    write_number(output_file_stream, std::get<Number>(literal));
                    break;
                case LiteralType::String:
                    write_string(output_file_stream, std::get<String>(literal));
                    break;
            }
        }
    }

    void CompiledProgram::write_names(std::basic_ofstream<char> &output_file_stream) const {
        for (auto& name : symbol_table->names) {
            write_name(output_file_stream, name);
        }
    }

    void CompiledProgram::write_header(std::ostream &output_file_stream, const ProgramHeader &header) const {
        for (char byte : header.magic) {
            write(output_file_stream, byte);
        }
        write(output_file_stream, header.timestamp);
        write(output_file_stream, header.name_count);
        write(output_file_stream, header.function_count);
        write(output_file_stream, header.literals_count);
    }


    void CompiledProgram::write_bytecode(std::ostream &output_file_stream) {
        for (uint8_t byte : main->bytecode) {
            write(output_file_stream, byte);
        }
    }

    BytecodeRunnerPtr CompiledProgram::create_runner(RuntimeEnvPtr runtime_env) {
        BytecodeRunnerPtr runner = std::make_shared<BytecodeRunner>(
                main->bytecode,
                symbol_table,
                runtime_env
        );

        std::vector<RuntimeValue> constants;
        constants.reserve(symbol_table->literals.size());
        for (auto& literal : symbol_table->literals) {
            LiteralType type = std::visit(LiteralTypeVisitor{}, literal);
            switch (type) {
                case LiteralType::Number:
                    constants.emplace_back(std::get<Number>(literal));
                    break;
                case LiteralType::String:
                    constants.emplace_back(std::get<String>(literal));
                    break;
            }
        }

        for (auto& function : symbol_table->functions) {

            std::vector<std::string> names;
            names.resize(function.second.names.size());
            for (auto& name_index : function.second.names) {
                names.emplace_back(symbol_table->get_name(name_index));
            }
            auto fun = std::make_shared<BytecodeFunction>(
                    runner,
                    function.second.bytecode,
                    names,
                    function.second.names.size() == 255
            );
            runtime_env->bind(function.first, std::dynamic_pointer_cast<Callable>(fun));
        }
        runner->set_constants(constants);
        return runner;
    }

    CallablePtr CompiledProgram::make_function_from_function_frame(std::shared_ptr<FunctionFrame> &frame, BytecodeRunnerPtr runner) {
        std::vector<std::string> argument_names;
        argument_names.reserve(frame->names.size());
        for (uint16_t index : frame->names) {
            argument_names.emplace_back(symbol_table->names[index]);
        }
        return std::dynamic_pointer_cast<Callable>(std::make_shared<BytecodeFunction>(
                    runner,
                    frame->bytecode,
                    argument_names,
                    argument_names.size() == 255
                ));
    }

    void CompiledProgram::write_functions(std::basic_ofstream<char> &output_file_stream) {
        for (auto& pair : symbol_table->functions) {

            auto& function = pair.second;
            FunctionInfo info{
                info.name_index = symbol_table->get_name_index(pair.first),
                static_cast<uint8_t>(function.names.size()),
                function.names,
                function.bytecode.size(),
                function.bytecode
            };
            write_function(output_file_stream, info);
        }
    }
}