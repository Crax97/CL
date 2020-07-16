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

void run_script(const std::string& script_path, Calculator::Env<Calculator::RuntimeValue>& env)
{
    std::ifstream file(script_path);
    std::string content, line;
    while (std::getline(file, line)) {
	content += line + "\n";
    }

    auto lexer = Calculator::Lexer(content);
    auto parser = Calculator::Parser(lexer);
    auto tree = parser.parse_all();
    auto evaluator = Calculator::ASTEvaluator(env);
    std::for_each(tree.begin(), tree.end(), [&evaluator](const Calculator::ExprPtr& ptr) {
	ptr->evaluate(evaluator);
    });
}

void run_from_cli(Calculator::Env<Calculator::RuntimeValue>& env)
{
    std::string line;
    std::cout << "> ";
    while (std::getline(std::cin, line)) {
	try {
	    auto lexer = Calculator::Lexer(line);
	    auto parser = Calculator::Parser(lexer);
	    auto tree = parser.parse_all();
	    auto evaluator = Calculator::ASTEvaluator(env);
	    if (tree.size() > 0) {
		std::for_each(tree.begin(), tree.end(), [&evaluator](const Calculator::ExprPtr& ptr) {
		    ptr->evaluate(evaluator);
		});
		std::cout << evaluator.get_result().to_string() << "\n";
	    }
	    std::cout << "> ";
	} catch (Calculator::CLException& ex) {
	    std::cerr << "Error: " << ex.get_message() << "\n";
	    std::cout << "> ";
	}
    }
}

int main(int argc, char** argv)
{
    auto env = Calculator::StackedEnvironment();
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
