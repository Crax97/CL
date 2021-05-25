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
#include "object.h"
#include "object_descriptor.h"

class TestClass_Descriptor : public CL::ObjectDescriptor {
    public:
        TestClass_Descriptor();


    virtual std::optional<CL::RuntimeValue> call(const CL::Args &args) override;
    virtual uint8_t arity() override {
        return 2;
    }
};

class TestClass : public CL::Object {
    friend class TestClass_Descriptor;
private:
    int integerMember;
    float floatMember;

public:
    TestClass(int inIntegerMember, float inFloatMember) 
        : Object(TestClass_Descriptor::get_descriptor<TestClass_Descriptor>()), integerMember{inIntegerMember}, floatMember{inFloatMember} {
    }

    double sum_members_with_third(double third) {
        return static_cast<double>(integerMember) + static_cast<float>(floatMember) + third; 
    }   

    int getIntegerMember() { return integerMember; }
    float getFloatMember() { return floatMember; }
};

TestClass_Descriptor::TestClass_Descriptor() 
    : ObjectDescriptor("TestClass") {
    bind_property("integer_member", &TestClass::integerMember);
    bind_property("float_member", &TestClass::floatMember);
    bind_method("sum_members_with_third", &TestClass::sum_members_with_third);
}

std::optional<CL::RuntimeValue> TestClass_Descriptor::call(const CL::Args& args)  {
    auto object = std::make_shared<TestClass>(static_cast<int>(args[0]), static_cast<double>(args[1]));
    return std::dynamic_pointer_cast<CL::Indexable>(object);
}

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

    SUBCASE("Testing classes support") {



        auto env = std::make_shared<CL::StackedEnvironment>();
        TestClass_Descriptor::get_descriptor<TestClass_Descriptor>()->bind_into_env(env);
        CL::inject_stdlib_functions(env);
        auto source = std::string(R"source(
            instance = TestClass(10, 1.0)
            instance.integer_member = 38
            instance.float_member = 3.0
            result = instance.sum_members_with_third(1.0)
        )source");
        CL::Script::from_source(source, env).run();
        auto instance = env->get("instance").as_object<TestClass>();
        auto result = env->get("result").as<double>();
        CHECK(instance->getIntegerMember() == 38);
        CHECK(result == 42.0);
    }
}