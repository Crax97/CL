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
#include "string_visitor.hpp"
#include "tokens.hpp"
#include "value.hpp"

void run_script(const std::string& script_path, Calculator::Env<Calculator::RuntimeValue>& env)
{
    std::ifstream file(script_path);
    std::string content;
    while (!file.eof()) {
	content += file.get();
    }

    auto parser = Calculator::Parser(Calculator::Lexer(content));
    auto tree = parser.parse_all();
    auto evaluator = Calculator::ASTEvaluator(env);
    std::for_each(tree.begin(), tree.end(), [&evaluator](const Calculator::ExprPtr& ptr) {
	ptr->evaluate(evaluator);
    });
    std::cout << evaluator.get_result().to_string() << "\n";
}

void run_from_cli(Calculator::Env<Calculator::RuntimeValue>& env)
{
    std::string line;
    std::cout << "> ";
    while (std::getline(std::cin, line)) {
	try {
	    auto parser = Calculator::Parser(Calculator::Lexer(line));
	    auto tree = parser.parse_all();
	    auto evaluator = Calculator::ASTEvaluator(env);
	    std::for_each(tree.begin(), tree.end(), [&evaluator](const Calculator::ExprPtr& ptr) {
		ptr->evaluate(evaluator);
	    });
	    std::cout << evaluator.get_result().to_string() << "\n";
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
    constexpr auto PI = 3.14159265359;
    env.assign("sin", Calculator::RuntimeValue(std::make_shared<Calculator::Function>([](const Calculator::Args& args, Calculator::Env<Calculator::RuntimeValue>& env) {
	auto num = args[0].as_number();
	return sin(num);
    },
			  1)));
    env.assign("cos", Calculator::RuntimeValue(std::make_shared<Calculator::Function>([](const Calculator::Args& args, auto& env) {
	auto num = args[0].as_number();
	return cos(num);
    },
			  1)));
    env.assign("deg2rad", Calculator::RuntimeValue(std::make_shared<Calculator::Function>([](const Calculator::Args& args, auto& env) {
	auto deg = args[0].as_number();
	return deg * PI / 180.0;
    },
			      1)));
    env.assign("rad2deg", Calculator::RuntimeValue(std::make_shared<Calculator::Function>([](const Calculator::Args& args, auto& env) {
	auto rad = args[0].as_number();
	return rad * 180.0 / PI;
    },
			      1)));
    env.assign("exit", Calculator::RuntimeValue(std::make_shared<Calculator::VoidFunction>([](const Calculator::Args& args, auto& env) {
	auto code = args[0].as_number();
	exit(static_cast<int>(code));
    },
			   1)));
    env.assign("PI", PI);

    if (argc == 1) {
	run_from_cli(env);
    } else
	for (int i = 1; i < argc; i++) {
	    run_script(argv[i], env);
	}
    return 0;
}
