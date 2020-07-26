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

class RangeIterator {
private:
    Number m_current, m_end, m_step;

public:
    RangeIterator(Number begin, Number end, Number step)
	: m_current(begin)
	, m_end(end)
	, m_step(step)
    {
    }

    bool has_next()
    {
	return m_current < m_end;
    }
    RuntimeValue next()
    {
	auto v = m_current;
	m_current += m_step;
	return RuntimeValue(v);
    }
};

void inject_import_function(RuntimeEnvPtr parent_env)
{
    static auto import_impl = std::make_shared<Function>(
	[parent_env](const Args& args) {
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
	    auto env = std::make_shared<StackedEnvironment>(parent_env);
	    ASTEvaluator evaluator(env);

	    for (const auto& expr : tree) {
		expr->evaluate(evaluator);
	    }
	    return evaluator.get_result();
	},
	1);
    parent_env->assign("import", RuntimeValue::make(import_impl));
}
void inject_stdlib_functions(RuntimeEnvPtr env)
{
    static auto exit_impl = std::make_shared<VoidFunction>([](const Calculator::Args& args) {
	auto code = args[0]->as_number();
	exit(static_cast<int>(code));
    },
	1);
    static auto input_impl = std::make_shared<Function>([](const auto& _args) {
	String line;
	std::getline(std::cin, line);
	return RuntimeValue(line);
    },
	0);
    static auto print_impl = std::make_shared<VoidFunction>([](const Args& args) {
	for (const auto& arg : args) {
	    std::cout << arg->to_string() << " ";
	}
	std::cout << "\n";
    },
	VAR_ARGS);

    static auto repr_impl = std::make_shared<Function>([](const Args& args) {
	return args[0]->string_representation();
    },
	1);

    static auto dict_impl = std::make_shared<Function>([](const Args& args) {
	auto dict = std::make_shared<Dictionary>();
	return RuntimeValue(std::dynamic_pointer_cast<Indexable>(dict));
    },
	VAR_ARGS);
    static auto range_impl = std::make_shared<Function>([](const Args& args) {
	auto begin = args[0]->as<Number>();
	auto end = args[1]->as<Number>();
	auto step = args[2]->as<Number>();

	auto iterator = std::make_shared<RangeIterator>(begin, end, step);
	auto dict = RuntimeValue(std::make_shared<Dictionary>());
	auto has_next_lambda = std::make_shared<Function>([iterator](const Args& args) {
	    return iterator->has_next();
	},
	    0);
	auto next_lambda = std::make_shared<Function>([iterator](const Args& args) {
	    return iterator->next();
	},
	    0);
	dict.set_named("__has_next", RuntimeValue(has_next_lambda));
	dict.set_named("__next", RuntimeValue(next_lambda));
	return dict;
    },
	3);
    env->assign("exit", RuntimeValue::make(exit_impl));
    env->assign("input", RuntimeValue::make(input_impl));
    env->assign("print", RuntimeValue::make(print_impl));
    env->assign("repr", RuntimeValue::make(repr_impl));
    env->assign("dict", RuntimeValue::make(dict_impl));
    env->assign("range", RuntimeValue::make(range_impl));
}
void inject_math_functions(RuntimeEnvPtr env)
{
    auto dict_object = RuntimeValue::make(std::make_shared<Dictionary>());
    static auto abs_impl = std::make_shared<Calculator::Function>([](const Calculator::Args& args) {
	auto num = args[0]->as_number();
	return num < 0 ? -num : num;
    },
	1);
    static auto sin_impl = std::make_shared<Calculator::Function>([](const Calculator::Args& args) {
	auto num = args[0]->as_number();
	return sin(num);
    },
	1);

    static auto cos_impl = std::make_shared<Calculator::Function>([](const Calculator::Args& args) {
	auto num = args[0]->as_number();
	return cos(num);
    },
	1);
    static auto deg2rad_impl = std::make_shared<Calculator::Function>([](const Calculator::Args& args) {
	auto deg = args[0]->as_number();
	return deg * PI / 180.0;
    },
	1);
    static auto rad2deg_impl = std::make_shared<Calculator::Function>([](const Calculator::Args& args) {
	auto rad = args[0]->as_number();
	return rad * 180.0 / PI;
    },
	1);
    dict_object->set_named("sin", RuntimeValue(sin_impl));
    dict_object->set_named("cos", RuntimeValue(cos_impl));
    dict_object->set_named("deg2rad", RuntimeValue(deg2rad_impl));
    dict_object->set_named("rad2deg", RuntimeValue(rad2deg_impl));
    dict_object->set_named("abs", RuntimeValue(abs_impl));
    dict_object->set_named("PI", PI);
    env->assign("Math", dict_object, true);
}
};
