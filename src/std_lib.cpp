#include "std_lib.hpp"
#include "ast_evaluator.hpp"
#include "commons.hpp"
#include "environment.hpp"
#include "exceptions.hpp"
#include "function_callable.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "value.hpp"

#include <cmath>
#include <fstream>
#include <memory>

constexpr auto PI = 3.14159265359;

namespace Calculator {

void inject_import_function(RuntimeEnv& env)
{
    // TODO: Change to Function when modules are ready
    static auto import_impl = std::make_shared<VoidFunction>(
	[](const Args& args, auto& env) {
	    String path = args[0]->as<String>();
	    auto file = std::fstream(path);
	    if (!file.is_open()) {
		throw RuntimeException("Cannot read file " + path);
	    }

	    std::string content, line;
	    while (std::getline(file, line)) {
		content += line;
	    }

	    auto parser = Parser(Lexer(content));
	    auto tree = parser.parse_all();
	    ASTEvaluator evaluator(env);

	    for (const auto& expr : tree) {
		evaluator.run_expression(expr);
	    }
	},
	1);
    env.assign("import", RuntimeValue::make(import_impl));
}
void inject_stdlib_functions(RuntimeEnv& env)
{
    static auto exit_impl = std::make_shared<Calculator::VoidFunction>([](const Calculator::Args& args, auto& env) {
	auto code = args[0]->as_number();
	exit(static_cast<int>(code));
    },
	1);
    static auto input_impl = std::make_shared<Function>([](const auto& _args, const auto& _env) {
	String line;
	std::getline(std::cin, line);
	return RuntimeValue(line);
    },
	0);
    static auto print_impl = std::make_shared<VoidFunction>([](const Args& args, const auto& _env) {
	for (const auto& arg : args) {
	    std::cout << arg->to_string() << " ";
	}
	std::cout << "\n";
    },
	VAR_ARGS);

    static auto repr_impl = std::make_shared<Function>([](const Args& args, const auto& _env) {
	return args[0]->string_representation();
    },
	1);

    env.assign("input", RuntimeValue::make(input_impl));
    env.assign("print", RuntimeValue::make(print_impl));
    env.assign("repr", RuntimeValue::make(repr_impl));
}
void inject_math_functions(RuntimeEnv& env)
{
    auto dict_object = RuntimeValue::make(RuntimeValue::make_dict());
    static auto sin_impl = std::make_shared<Calculator::Function>([](const Calculator::Args& args, auto& env) {
	auto num = args[0]->as_number();
	return sin(num);
    },
	1);

    static auto cos_impl = std::make_shared<Calculator::Function>([](const Calculator::Args& args, auto& env) {
	auto num = args[0]->as_number();
	return cos(num);
    },
	1);
    static auto deg2rad_impl = std::make_shared<Calculator::Function>([](const Calculator::Args& args, auto& env) {
	auto deg = args[0]->as_number();
	return deg * PI / 180.0;
    },
	1);
    static auto rad2deg_impl = std::make_shared<Calculator::Function>([](const Calculator::Args& args, auto& env) {
	auto rad = args[0]->as_number();
	return rad * 180.0 / PI;
    },
	1);
    dict_object->set_named("sin", RuntimeValue(sin_impl));
    dict_object->set_named("cos", RuntimeValue(cos_impl));
    dict_object->set_named("deg2rad", RuntimeValue(deg2rad_impl));
    dict_object->set_named("rad2deg", RuntimeValue(rad2deg_impl));
    env.assign("PI", RuntimeValue::make(PI));
    env.assign("Math", dict_object);
}
};
