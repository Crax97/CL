#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <istream>
#include <memory>
#include <string>

#include "ast_evaluator.hpp"
#include "commons.hpp"
#include "environment.hpp"
#include "exceptions.hpp"
#include "function_callable.hpp"
#include "lexer.hpp"
#include "nodes.hpp"
#include "parser.hpp"
#include "std_lib.hpp"
#include "string_visitor.hpp"
#include "tokens.hpp"
#include "value.hpp"

void run_script(const std::string& script_path, std::shared_ptr<Calculator::StackedEnvironment> env)
{
    std::ifstream file(script_path);
    std::string content, line;
    while (std::getline(file, line)) {
	content += line + "\n";
    }

    auto parser = Calculator::Parser(Calculator::Lexer(content));
    auto tree = parser.parse_all();
    auto evaluator = Calculator::ASTEvaluator(env);
    std::for_each(tree.begin(), tree.end(), [&evaluator](const Calculator::ExprPtr& ptr) {
	ptr->evaluate(evaluator);
    });
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

void run_from_cli(std::shared_ptr<Calculator::StackedEnvironment> env)
{
    while (true) {
	try {
	    auto line = read_from_console();
	    auto parser = Calculator::Parser(Calculator::Lexer(line));
	    auto tree = parser.parse_all();
	    auto evaluator = Calculator::ASTEvaluator(env);
	    if (tree.size() > 0) {
		std::for_each(tree.begin(), tree.end(), [&evaluator](const Calculator::ExprPtr& ptr) {
		    ptr->evaluate(evaluator);
		});

		if (evaluator.has_value()) {
		    std::cout << evaluator.get_result().to_string() << "\n";
		}
	    }
	} catch (Calculator::CLException& ex) {
	    std::cerr << "Error: " << ex.get_message() << "\n";
	    std::cout << "> ";
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
