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
        LiteralType operator()(const std::shared_ptr<CL::FunctionFrame>& ) { return LiteralType::Function; }
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
void write_function(std::ofstream& output_file_stream, std::shared_ptr<FunctionFrame> &function) {
    char type_byte = static_cast<char>(LiteralType::Function);
    output_file_stream.write(&type_byte, 1);
    write<uint8_t>(output_file_stream, function->names.size());
    write<uint64_t>(output_file_stream, function->bytecode_count());
    for (uint16_t name : function->names) {
        write(output_file_stream, name);
    }
    for (uint8_t byte : function->bytecode) {
        write(output_file_stream, byte);
    }
}

void CompiledProgram::write_to_file(const std::string &file_path) {
    auto path = std::filesystem::path(file_path);
    auto output_file_stream = std::ofstream(path, std::ios_base::out);
    if (!(path.has_filename() && output_file_stream.is_open())) { throw FileException(file_path); }

    ProgramHeader header;
    header.timestamp = std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
    header.name_count = names.size();
    header.literals_count = literals.size();

    write_header(output_file_stream, header);
    write_names(output_file_stream);
    write_literals(output_file_stream);
    write_bytecode(output_file_stream);
}

    void CompiledProgram::write_literals(std::basic_ofstream<char> &output_file_stream) {
        for (auto& literal : literals) {
            LiteralType literal_type = std::visit(LiteralTypeVisitor{}, literal);
            switch(literal_type) {
                case LiteralType::Number:
                    write_number(output_file_stream, std::get<Number>(literal));
                    break;
                case LiteralType::String:
                    write_string(output_file_stream, std::get<String>(literal));
                    break;
                case LiteralType::Function:
                    write_function(output_file_stream, std::get<std::shared_ptr<FunctionFrame>>(literal));
                    break;
            }
        }
    }

    void CompiledProgram::write_names(std::basic_ofstream<char> &output_file_stream) const {
        for (auto& name : names) {
            write_name(output_file_stream, name);
        }
    }

    void CompiledProgram::write_header(std::ostream &output_file_stream, const ProgramHeader &header) const {
        for (char byte : header.magic) {
            write(output_file_stream, byte);
        }
        write(output_file_stream, header.timestamp);
        write(output_file_stream, header.name_count);
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
                names,
                std::make_shared<StackedEnvironment>(runtime_env)
        );

        std::vector<RuntimeValue> constants;
        constants.reserve(literals.size());
        for (auto& literal : literals) {
            LiteralType type = std::visit(LiteralTypeVisitor{}, literal);
            switch (type) {
                case LiteralType::Number:
                    constants.emplace_back(std::get<Number>(literal));
                    break;
                case LiteralType::String:
                    constants.emplace_back(std::get<String>(literal));
                    break;
                case LiteralType::Function:
                    constants.emplace_back(make_function_from_function_frame(std::get<std::shared_ptr<FunctionFrame>>(literal), runner));
                    break;
            }
        }
        runner->set_constants(constants);
        return runner;
    }

    CallablePtr CompiledProgram::make_function_from_function_frame(std::shared_ptr<FunctionFrame> &frame, BytecodeRunnerPtr runner) {
        std::vector<std::string> argument_names;
        argument_names.reserve(frame->names.size());
        for (uint16_t index : frame->names) {
            argument_names.emplace_back(names[index]);
        }
        return std::dynamic_pointer_cast<Callable>(std::make_shared<BytecodeFunction>(
                    runner,
                    frame->bytecode,
                    argument_names,
                    argument_names.size() == 255
                ));
    }
}