//
// Created by gsoli on 17/12/2020.
//

#pragma once

#include "nodes.hpp"
#include "commons.hpp"
#include "stack_based_evaluator.hpp"

namespace CL {

    enum class LiteralType : uint8_t {
        Number = 0x01,
        String = 0x02,
    };

    /*
     * File layout:
     *
     * HEADER (20 bytes)
     * names: name_count null-terminated strings representing the names
     * literals: literal_counts (type, value) pairs.
     *      if type == Number then value is a 64 bit double
     *      if type == String the value is a null terminated string
     *
     * functions: 32 bit integer with the number of (name_index, arg_count, args, code_size, code) tuples:
     *      name_index is the 16 bit index of this function's name in the names array
     *      arg_count is the 8 bit count of argument indices
     *      args is an array of 16 bit indices of arg_count length
     *      code_size is a 64 bit unsigned integer containing this function's bytecode length
     *      code is the bytecode of the function
     *  main_code
     * */
    struct FunctionInfo {
        uint16_t name_index {};
        uint8_t arg_count {};
        std::vector<uint16_t> args{};
        uint64_t code_size {};
        std::vector<uint8_t> bytecode{};
    };
    struct ProgramHeader {
        // This must be "BADCODE\0"
        char magic[8] ="BADCODE";
        uint64_t timestamp {};
        // Defines how many names the program contains
        uint16_t name_count {};
        // Defines how many literal values are present in the program
        uint32_t function_count {};
        // Defines how many literal values are present in the program
        uint32_t literals_count {};
    };

    class CompiledProgram {
    private:
        void write_header(std::ostream &output_file_stream, const ProgramHeader &header) const;
        void write_names(std::basic_ofstream<char> &output_file_stream) const;
        void write_literals(std::basic_ofstream<char> &output_file_stream);
        void write_functions(std::basic_ofstream<char> &output_file_stream);
        void write_bytecode(std::ostream &output_file_stream);
    public:
        std::shared_ptr<CompilationStackFrame> main;
        SymbolTablePtr symbol_table;

        BytecodeRunnerPtr create_runner(RuntimeEnvPtr runtime_env);
        void write_to_file(const std::string& file_path);
        CallablePtr make_function_from_function_frame(std::shared_ptr<FunctionFrame> &frame, BytecodeRunnerPtr runner);
    };
} // NAMESPACE CL