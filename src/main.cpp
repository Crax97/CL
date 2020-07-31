#include <algorithm>
#include <fstream>
#include <iostream>
#include <istream>
#include <memory>
#include <string>

#include "ast_evaluator.hpp"
#include "commons.hpp"
#include "environment.hpp"
#include "exceptions.hpp"
#include "std_lib.hpp"
#include "script.h"

void run_script(const std::string& script_path, std::shared_ptr<Calculator::StackedEnvironment> env)
{
    auto script = Calculator::Script::from_file(script_path, env);
    script.run();
}

std::string read_from_console()
{
    std::string content, line;
    constexpr std::string_view SCOPE_IN = "{";
    constexpr std::string_view SCOPE_OUT = "}";
    int scope = 0;
    do {
	std::cout << std::to_string(scope) << "> ";
	std::getline(std::cin, line);

	for (char c : line) {
	    if (SCOPE_IN.find(c) != std::string_view::npos)
		scope++;
	    if (SCOPE_OUT.find(c) != std::string_view::npos)
		scope--;
	}
	content += line;
    } while (scope > 0);
    return content;
}

void run_from_cli(const Calculator::RuntimeEnvPtr& env)
{
    while (true) {
        try {
            auto source = read_from_console();
            auto script = Calculator::Script::from_source(source, env);
            auto result = script.run();
            if (result.has_value()) {
                std::cout << result.value().to_string() << "\n";
            }
        } catch (Calculator::CLException& ex) {
            std::cerr << "Error: " << ex.get_message() << "\n";
        }
    }
}

int main(int argc, char** argv)
{
    auto env = std::make_shared<Calculator::StackedEnvironment>();
    Calculator::inject_import_function(env);
    Calculator::inject_math_functions(env);
    Calculator::inject_stdlib_functions(env);
    if (argc == 1) {
	run_from_cli(env);
    } else
	for (int i = 1; i < argc; i++) {
	    run_script(argv[i], env);
	}
    return 0;
}
