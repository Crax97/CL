#include "std_lib.hpp"
#include "ast_evaluator.hpp"
#include "commons.hpp"
#include "environment.hpp"
#include "exceptions.hpp"
#include "function_callable.hpp"
#include "value.hpp"
#include "script.h"
#include "helpers.h"

#include <cmath>
#include <fstream>
#include <memory>
#include <functional>

constexpr auto PI = 3.14159265359;

namespace CL {
std::optional<RuntimeValue> exit_impl(int code) {
    exit(code);
    return std::nullopt;
}

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

    [[nodiscard]]
    bool has_next() const
    {
	return m_current < m_end;
    }

    [[nodiscard]]
    RuntimeValue next()
    {
	auto v = m_current;
	m_current += m_step;
	return RuntimeValue(v);
    }
};

class FileIterator {
private:
    std::ifstream m_stream;
public:
    explicit FileIterator(std::ifstream&& stream) :
    m_stream(std::move(stream))
    {
        if(!m_stream.is_open())
            throw RuntimeException("Tried opening a bad ifstream");
    }
    [[nodiscard]]
    bool has_next() const
    {
        return !m_stream.eof();
    }

    [[nodiscard]]
    RuntimeValue next()
    {
        std::string line;
        std::getline(m_stream, line);
        return line;
    }
};

std::optional<RuntimeValue> import_impl(std::string path, const RuntimeEnvPtr& env) {
    auto script = Script::from_file(path, env);
    return script.run();
}

void inject_import_function(const RuntimeEnvPtr& parent_env)
{
    static std::function fn =  [parent_env] (std::string str){ return import_impl(str, parent_env); };
    static auto fun_ptr = CL::make_function(fn);
    parent_env->assign("import", RuntimeValue(fun_ptr));
}
void inject_stdlib_functions(const RuntimeEnvPtr& env)
{
    static auto input_impl = std::make_shared<LambdaStyleFunction>([](const auto& _args) {
	String line;
	std::getline(std::cin, line);
	return RuntimeValue(line);
    },
                                                                   0);
    static auto print_impl = std::make_shared<VoidFunction>([](const Args& args) {
	for (const auto& arg : args) {
	    std::cout << arg.to_string() << " ";
	}
	std::cout << "\n";
    },
	VAR_ARGS);

    static auto repr_impl = std::make_shared<LambdaStyleFunction>([](const Args& args) {
	return args[0].string_representation();
    },
                                                                  1);

    static auto range_impl = std::make_shared<LambdaStyleFunction>([](const Args& args) {
	auto begin = args[0].as<Number>();
	auto end = args[1].as<Number>();
	auto step = args[2].as<Number>();

	auto iterator = std::make_shared<RangeIterator>(begin, end, step);
	auto dict = RuntimeValue(std::make_shared<Dictionary>());
	auto has_next_lambda = std::make_shared<LambdaStyleFunction>([iterator](const Args& args) {
	    return iterator->has_next();
	},
                                                                 0);
	auto next_lambda = std::make_shared<LambdaStyleFunction>([iterator](const Args& args) {
	    return iterator->next();
	},
                                                             0);
	dict.set_named("__has_next", RuntimeValue(has_next_lambda));
	dict.set_named("__next", RuntimeValue(next_lambda));
	return dict;
    },
                                                                   3);

    static auto open_impl = std::make_shared<LambdaStyleFunction>([](const Args& args){
            auto file_path = args[0].as<String>();
            auto stream = std::ifstream(file_path);
            auto file_it = std::make_shared<FileIterator>(std::move(stream));

            auto dict = RuntimeValue(std::make_shared<Dictionary>());
            auto has_next_lambda = std::make_shared<LambdaStyleFunction>([file_it](const Args& args) {
                                                                  return file_it->has_next();
                                                              },
                                                                         0);
            auto next_lambda = std::make_shared<LambdaStyleFunction>([file_it](const Args& args) {
                                                              return file_it->next();
                                                          },
                                                                     0);
            dict.set_named("__has_next", RuntimeValue(has_next_lambda));
            dict.set_named("__next", RuntimeValue(next_lambda));
            return dict;
        }, 1);
    env->assign("exit", RuntimeValue(&exit_impl));
    env->assign("input", RuntimeValue(input_impl));
    env->assign("print", RuntimeValue(print_impl));
    env->assign("repr", RuntimeValue(repr_impl));
    env->assign("range", RuntimeValue(range_impl));
    env->assign("open", RuntimeValue(open_impl));
}
void inject_math_functions(const RuntimeEnvPtr& env)
{
    auto dict_object = RuntimeValue(std::make_shared<Dictionary>());
    static auto abs_impl = std::make_shared<CL::LambdaStyleFunction>([](const CL::Args& args) {
	auto num = args[0].as<Number>();
	return num < 0 ? -num : num;
    },
                                                                     1);
    static auto sin_impl = std::make_shared<CL::LambdaStyleFunction>([](const CL::Args& args) {
	auto num = args[0].as<Number>();
	return sin(num);
    },
                                                                     1);

    static auto cos_impl = std::make_shared<CL::LambdaStyleFunction>([](const CL::Args& args) {
	auto num = args[0].as<Number>();
	return cos(num);
    },
                                                                     1);
    static auto deg2rad_impl = std::make_shared<CL::LambdaStyleFunction>([](const CL::Args& args) {
	auto deg = args[0].as<Number>();
	return deg * PI / 180.0;
    },
                                                                         1);
    static auto rad2deg_impl = std::make_shared<CL::LambdaStyleFunction>([](const CL::Args& args) {
	auto rad = args[0].as<Number>();
	return rad * 180.0 / PI;
    },
                                                                         1);
    dict_object.set_named("sin", RuntimeValue(sin_impl));
    dict_object.set_named("cos", RuntimeValue(cos_impl));
    dict_object.set_named("deg2rad", RuntimeValue(deg2rad_impl));
    dict_object.set_named("rad2deg", RuntimeValue(rad2deg_impl));
    dict_object.set_named("abs", RuntimeValue(abs_impl));
    dict_object.set_named("PI", PI);
    env->assign("Math", dict_object, true);
}
};
