#include <algorithm>
#include <fstream>
#include <iostream>
#include <istream>
#include <memory>
#include <string>

#include "vm_ast_evaluator.h"
#include "commons.hpp"
#include "environment.hpp"
#include "exceptions.hpp"
#include "lexer.hpp"
#include "std_lib.hpp"
#include "script.h"
#include "parser.hpp"

void print_program(CL::Program &program);

void run_script(const std::string &script_path,
                std::shared_ptr<CL::StackedEnvironment> env) {
	auto script = CL::Script::from_file(script_path, env);
	script.run();
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
	while (true) {
		try {
			auto source = read_from_console();
            auto stream = std::stringstream(source);
            auto lexer = CL::Lexer(stream);
            auto parser = CL::Parser(lexer);
            auto expressions = parser.parse_all();
            CL::VMASTEvaluator compiler;
            for(auto& expr : expressions) {
                expr->evaluate(compiler);
            }
            CL::Program program = compiler.get_program();
            print_program(program);

        } catch (CL::CLException &ex) {
			std::cerr << "Error: " << ex.get_message() << "\n";
		}
	}
}
std::string print_bytecode(int num_tabs, std::vector<uint8_t> &bytecode) {
    std::stringstream bytecode_string;
    for (auto byte : bytecode) {
        bytecode_string << std::string(num_tabs, '\t') << "0x" <<
                        std::hex << std::uppercase << (int) byte << std::nouppercase << std::dec << "\n";
    }
    return bytecode_string.str();
}
void print_program(CL::Program &program) {
    struct literal_visitor {
        std::string operator()(const CL::Number n) { return std::to_string(n); }
        std::string operator()(const CL::String& s) { return s; }
        std::string operator()(const std::shared_ptr<CL::FunctionFrame> frame) {
            std::stringstream name_stream;
            name_stream << "Function (";
            for (auto& name : frame->names) {
                name_stream << name << ",";
            }
            name_stream << ")\n";
            return name_stream.str() + print_bytecode(3, frame->bytecode); }
    };
    std::cout << "Program info: \n";
    std::cout << "\t" << program.names.size() << " names\n";
    for (auto& name : program.names) {
        std::cout << "\t\t " << name << "\n";
    }
    std::cout << "\t" << program.literals.size() << " literals\n";
    for (auto& literal : program.literals) {
        std::cout << "\t\t " << std::visit(literal_visitor{}, literal) << "\n";
    }
    std::cout << "\t" << program.functions.size() << " functions\n";

    for (auto& function : program.functions) {
        std::cout << "\t\t (";
        for(auto& name : function->names) {
            std::cout << name << ", ";
        }
        std::cout << ")\n";
    }

    std::cout << "\tmain section:\n" << print_bytecode(1, program.main->bytecode);

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
			run_script(argv[i], env);
		}
	return 0;
}
