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

    class RangeIterator {
    private:
        Number m_current, m_end, m_step;

    public:
        RangeIterator(Number begin, Number end, Number step)
                : m_current(begin), m_end(end), m_step(step) {
        }

        [[nodiscard]]
        bool has_next() const {
            return m_current < m_end;
        }

        [[nodiscard]]
        RuntimeValue next() {
            auto v = m_current;
            m_current += m_step;
            return RuntimeValue(v);
        }
    };

    class FileIterator {
    private:
        std::ifstream m_stream;
    public:
        explicit FileIterator(std::ifstream &&stream) :
                m_stream(std::move(stream)) {
            if (!m_stream.is_open())
                throw RuntimeException("Tried opening a bad ifstream");
        }

        [[nodiscard]]
        bool has_next() const {
            return !m_stream.eof();
        }

        [[nodiscard]]
        RuntimeValue next() {
            std::string line;
            std::getline(m_stream, line);
            return line;
        }
    };

    std::optional<RuntimeValue> import_impl(const std::string& path, const RuntimeEnvPtr &env) {
        auto script = Script::from_file(path, env);
        return script.run();
    }

    std::optional<RuntimeValue> range(Number begin, Number end, Number step) {
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
    }

    std::optional<RuntimeValue> open(const std::string& file_path) {
        auto stream = std::ifstream(file_path);
        auto file_it = std::make_shared<FileIterator>(std::move(stream));

        auto dict = RuntimeValue(std::make_shared<Dictionary>());
        auto has_next_lambda = std::make_shared<LambdaStyleFunction>([file_it](const Args &args) {
                                                                         return file_it->has_next();
                                                                     },
                                                                     0);
        auto next_lambda = std::make_shared<LambdaStyleFunction>([file_it](const Args &args) {
                                                                     return file_it->next();
                                                                 },
                                                                 0);
        dict.set_named("__has_next", RuntimeValue(has_next_lambda));
        dict.set_named("__next", RuntimeValue(next_lambda));
        return dict;
    }
    Number deg2rad(double deg) {
        return deg * PI / 180.0;
    }
    Number rad2deg(double rad) {
        return rad * 180.0 / PI;
    }

    void inject_import_function(const RuntimeEnvPtr &parent_env) {
        static std::function fn = [parent_env](const std::string& str) { return import_impl(str, parent_env); };
        static auto fun_ptr = CL::make_function(fn);
        parent_env->assign("import", RuntimeValue(fun_ptr));
    }

    void inject_stdlib_functions(const RuntimeEnvPtr &env) {
        auto exit_impl = CL::make_function(exit);
        auto range_impl = CL::make_function(range);
        auto open_impl = CL::make_function(open);
        static auto input_impl = std::make_shared<LambdaStyleFunction>([](const auto &_args) {
                                                                           String line;
                                                                           std::getline(std::cin, line);
                                                                           return RuntimeValue(line);
                                                                       },
                                                                       0);
        static auto print_impl = std::make_shared<VoidFunction>([](const Args &args) {
                                                                    for (const auto &arg : args) {
                                                                        std::cout << arg.to_string() << " ";
                                                                    }
                                                                    std::cout << "\n";
                                                                },
                                                                VAR_ARGS);

        static auto repr_impl = std::make_shared<LambdaStyleFunction>([](const Args &args) {
                                                                          return args[0].string_representation();
                                                                      },
                                                                      1);
        env->assign("exit", exit_impl);
        env->assign("input", RuntimeValue(input_impl));
        env->assign("print", RuntimeValue(print_impl));
        env->assign("repr", RuntimeValue(repr_impl));
        env->assign("range", range_impl);
        env->assign("open", open_impl);
    }


    void inject_math_functions(const RuntimeEnvPtr &env) {
        auto dict_object = RuntimeValue(std::make_shared<Dictionary>());

        dict_object.set_named("sin", CL::make_function(sin));
        dict_object.set_named("cos", CL::make_function(cos));
        dict_object.set_named("deg2rad", CL::make_function(deg2rad));
        dict_object.set_named("rad2deg", CL::make_function(rad2deg));
        dict_object.set_named("abs", CL::make_function(abs));
        dict_object.set_named("PI", PI);
        env->assign("Math", dict_object, true);
    }
};
