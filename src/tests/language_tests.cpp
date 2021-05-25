//
// Created by Crax on 22/12/2020.
//

#include "doctest.h"

#include <optional>
#include <memory>

#include "script.h"
#include "value.hpp"
#include "environment.hpp"
#include "std_lib.hpp"

TEST_CASE("Testing language constructs with the AST evaluator") {
    SUBCASE("Testing simple expression") {
        auto source = "value = (8 - 1 + 3) * 6 - ((3 + 7) * 2)";
        auto env = std::make_shared<CL::StackedEnvironment>();
        CL::Script::from_source(source, env).run();
        auto value = env->get("value");
        CHECK(value.as<CL::Number>() == (8 - 1 + 3) * 6 - ((3 + 7) * 2));
    }

    SUBCASE("Testing for") {
        auto source = std::string(R"source(
        value = 0
        for i in range(0, 10, 1) {
            value = value + i
        }
        value
        )source");
        auto env = std::make_shared<CL::StackedEnvironment>();
        CL::inject_stdlib_functions(env);
        CL::Script::from_source(source, env).run();
        auto value = env->get("value");
            CHECK(value.as<CL::Number>() == 9 * 10 / 2);
    }

    SUBCASE("Testing while") {
        auto source = std::string(R"source(
        value = 0
        i = 0
        while i < 10 {
            value = value + i
            i = i + 1
        }
        value
        )source");
        auto env = std::make_shared<CL::StackedEnvironment>();
        CL::inject_stdlib_functions(env);
        CL::Script::from_source(source, env).run();
        auto value = env->get("value");
            CHECK(value.as<CL::Number>() == 9 * 10 / 2);
    }

    SUBCASE("Testing if 1") {
        auto source = std::string(R"source(
        value = 0
        x = 1
        y = 2
        if (x < y) {
            value = "yes"
        } else {
            value = "no"
        })source");
        auto env = std::make_shared<CL::StackedEnvironment>();
        CL::inject_stdlib_functions(env);
        CL::Script::from_source(source, env).run();
        auto value = env->get("value");
        CHECK(value.as<CL::String>() == "yes");
    }

            SUBCASE("Testing if 2") {
        auto source = std::string(R"source(
        value = 0
        x = 1
        y = 2
        if (x > y) {
            value = "yes"
        } else {
            value = "no"
        })source");
        auto env = std::make_shared<CL::StackedEnvironment>();
        CL::inject_stdlib_functions(env);
        CL::Script::from_source(source, env).run();
        auto value = env->get("value");
                CHECK(value.as<CL::String>() == "no");
    }

    SUBCASE("Testing functions 1") {
        auto source = std::string(R"source(
        function forty_two() {
            return 42
        }
        value = forty_two()
        )source");
        auto env = std::make_shared<CL::StackedEnvironment>();
        CL::inject_stdlib_functions(env);
        CL::Script::from_source(source, env).run();
        auto value = env->get("value");
        CHECK(value.as<CL::Number>() == 42);
    }

    SUBCASE("Testing functions 2") {
        auto source = std::string(R"source(
        function divide(x, y) {
            return x / y
        }
        )source");
        auto env = std::make_shared<CL::StackedEnvironment>();
        CL::inject_stdlib_functions(env);
        CL::Script::from_source(source, env).run();
        auto function = env->get("divide");
        CHECK(function.as<CL::CallablePtr>()->call({10, 5}) == 2);
    }
}