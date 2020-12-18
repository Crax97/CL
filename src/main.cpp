#include <algorithm>
#include <iostream>
#include <istream>
#include <memory>
#include <string>
#include <utility>
#include <filesystem>
#include <fstream>

#include "vm_ast_evaluator.h"
#include "bytecode_runner.hpp"
#include "environment.hpp"
#include "lexer.hpp"
#include "std_lib.hpp"
#include "script.h"
#include "parser.hpp"
#include "program.h"

std::string print_bytecode(int num_tabs, std::vector<uint8_t> &bytecode) {
    using namespace CL;
    std::stringstream bytecode_string;
    int i = 0;
    auto read8 = [&]() { return bytecode[i++]; };
    auto read16 = [&]() { return (uint16_t)(read8()) << 8 | read8(); };
    auto read32 = [&]() { return (uint16_t)(read8()) << 24 | (uint16_t)(read8()) << 16
                                 | (uint16_t)(read8()) << 8 | read8(); };
    auto print_with_bytes8 = [&](uint8_t val) {
        bytecode_string << val << "\n";
        bytecode_string << std::string(num_tabs, '\t') << std::hex << "0x"
                        << (int)val << std::dec << "\n";
    };
    auto print_with_bytes16 = [&](uint16_t val) {
        bytecode_string << val << "\n";
        bytecode_string << std::string(num_tabs, '\t') << std::hex << "0x"
                        << (int) (val >> 8) << std::dec << "\n";
        bytecode_string << std::string(num_tabs, '\t') << std::hex << "0x"
                        << (int) (val & 0x00FF) << std::dec << "\n";
    };
    auto print_with_bytes32 = [&](uint32_t val) {
        bytecode_string << val << "\n";
        bytecode_string << std::string(num_tabs, '\t') << "0x" << std::hex
                        << (int) (val >> 24) << std::dec << "\n";
        bytecode_string << std::string(num_tabs, '\t') << "0x" << std::hex
                        << (int) (val >> 16) << std::dec << "\n";
        bytecode_string << std::string(num_tabs, '\t') << "0x" << std::hex
                        << (int) (val >> 8) << std::dec << "\n";
        bytecode_string << std::string(num_tabs, '\t') << "0x" << std::hex
                        << (int) (val & 0x000000FF) << std::dec << "\n";
    };
    while(i < bytecode.size()) {
        uint8_t byte = read8();
        Opcode op = byte_to_opcode(byte);
        bytecode_string << std::string(num_tabs, '\t') << "0x" <<
                        std::hex << std::uppercase << (int) byte << std::nouppercase << std::dec << " "
                        << opcode_to_string(op) << " ";
        switch (op) {
            case Opcode::Load_Literal:
                print_with_bytes32(read32());
                break;
            case Opcode::Load:
            case Opcode::Store:
            case Opcode::List:
                print_with_bytes16(read16());
                break;
            case Opcode::Dict:
                bytecode_string << "2 * ";
                print_with_bytes16(read16());
                break;
            case Opcode::Call: {
                bytecode_string << " with arg count: ";
                uint8_t arg_count = read8();
                if (arg_count == UINT8_MAX) {
                    bytecode_string << "Variadic";
                } else {
                    print_with_bytes8(arg_count);
                }
            }
                break;
            default:
                bytecode_string << "\n";
                break;
        }
    }
    return bytecode_string.str();
}
void print_program(CL::CompiledProgram &program) {
    struct literal_visitor {
    private:
        CL::CompiledProgram& program_reference;
    public:
        explicit literal_visitor(CL::CompiledProgram& in_program_reference)
                : program_reference(in_program_reference) {}
        std::string operator()(const CL::Number n) { return std::to_string(n); }
        std::string operator()(const CL::String& s) { return s; }
        std::string operator()(const std::shared_ptr<CL::FunctionFrame>& frame) {
            std::stringstream name_stream;
            name_stream << "Function (";
            for (auto& name_index : frame->names) {
                name_stream << program_reference.symbol_table->names[name_index] << ",";
            }
            name_stream << ")\n";
            return name_stream.str() + print_bytecode(3, frame->bytecode); }
    };
    std::cout << "CompiledProgram info: \n";
    std::cout << "\t" << program.symbol_table->names.size() << " names\n";
    int i = 0;
    for (auto& name : program.symbol_table->names) {
        std::cout << "\t\t " << i++ << " | " << name << "\n";
    }
    i = 0;
    std::cout << "\t" << program.symbol_table->literals.size() << " literals\n";
    for (auto& literal : program.symbol_table->literals) {
        std::cout << "\t\t " << i++ << " | " << std::visit(literal_visitor{program}, literal) << "\n";
    }

    std::cout << "\tmain section:\n" << print_bytecode(2, program.main->bytecode);

}
void compile_program(const std::filesystem::path& path){
    auto stream = std::fstream(path, std::fstream::in);
    auto lexer = CL::Lexer(stream);
    auto parser = CL::Parser(lexer);
    auto expressions = parser.parse_all();
    CL::VMASTEvaluator compiler(std::make_shared<CL::SymbolTable>());
    for(auto& expr : expressions) {
        expr->evaluate(compiler);
    }
    CL::CompiledProgram program = compiler.get_program();

    std::string filename = path.filename().generic_string();
    int dot_pos = filename.find_last_of('.');
    if (dot_pos != std::string::npos) {
        filename = filename.substr(0, dot_pos);
    }
    filename.append(".clc");
    program.write_to_file(filename);
}

std::string read_from_console() {
	std::string content, line;
	constexpr std::string_view SCOPE_IN = "{";
	constexpr std::string_view SCOPE_OUT = "}";
	int scope = 0;
	do {
		std::cout << std::to_string(scope) << "> ";
		std::getline(std::cin, line);

		for (char c : line) {
			if(SCOPE_IN.find(c) != std::string_view::npos)
				scope++;
			if(SCOPE_OUT.find(c) != std::string_view::npos)
				scope--;
		}
		content += line;
	} while (scope > 0);
	return content;
}

void run_from_cli(const CL::RuntimeEnvPtr &env) {
    CL::SymbolTablePtr global_symbols = std::make_shared<CL::SymbolTable>();
	while (true) {
		try {
			auto source = read_from_console();
            auto stream = std::stringstream(source);
            auto lexer = CL::Lexer(stream);
            auto parser = CL::Parser(lexer);
            auto expressions = parser.parse_all();
            CL::VMASTEvaluator compiler(global_symbols);
            for(auto& expr : expressions) {
                expr->evaluate(compiler);
            }
            CL::CompiledProgram program = compiler.get_program();
            CL::BytecodeRunnerPtr runner = program.create_runner(env);
            auto result = runner->run();
            if (result.has_value()) {
                std::cout << result->to_string();
            }

        } catch (CL::CLException &ex) {
			std::cerr << "Error: " << ex.get_message() << "\n";
		}
	}
}

int main(int argc, char **argv) {
	auto env = std::make_shared<CL::StackedEnvironment>();
	CL::inject_import_function(env);
	CL::inject_math_functions(env);
	CL::inject_stdlib_functions(env);
	if(argc == 1) {
		run_from_cli(env);
	} else
		for (int i = 1; i < argc; i++) {
			compile_program(argv[i]);
		}
	return 0;
}
