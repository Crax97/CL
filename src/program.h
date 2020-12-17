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
        Function = 0x03,
    };

    /*
     * File layout:
     *
     * HEADER (20 bytes)
     * names: name_count null-terminated strings representing the names
     * literals: literal_counts (type, value) pairs.
     *      if type == Number then value is a 64 bit double
     *      if type == String the value is a null terminated string
     *      if type == Function then value is a (fun_names, fun_length, name_indices, code) pair where:
     *         fun_names is an 8-bit unsigned integer representing the length of the name_indices array
     *         fun_length is a 64bit value representing the length of the code array
     *         name_indices is an array of 16 bit values of length fun_names that represents the indices of the "names" array
     *         code is the code of the function
     *  main_code
     * */
    struct ProgramHeader {
        // This must be "BADCODE\0"
        char magic[8] ="BADCODE";
        uint64_t timestamp;
        // Defines how many names the program contains
        uint16_t name_count;
        // Defines how many literal values are present in the program
        uint32_t literals_count;
    };

    class CompiledProgram {
    private:
        void write_header(std::ostream &output_file_stream, const ProgramHeader &header) const;
        void write_names(std::basic_ofstream<char> &output_file_stream) const;
        void write_literals(std::basic_ofstream<char> &output_file_stream);
        void write_bytecode(std::ostream &output_file_stream);
    public:
        std::shared_ptr<CompilationStackFrame> main;
        std::deque<std::string> names;
        std::deque<LiteralValue> literals;

        BytecodeRunnerPtr create_runner(RuntimeEnvPtr runtime_env);
        void write_to_file(const std::string& file_path);
        CallablePtr make_function_from_function_frame(std::shared_ptr<FunctionFrame> &frame, BytecodeRunnerPtr runner);
    };
} // NAMESPACE CL