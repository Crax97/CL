#pragma once

#include "commons.hpp"
#include "value.hpp"

#include <sstream>

namespace Calculator {
class VoidFunction : public Callable {
private:
    VoidFunctionCallback m_function;
    uint8_t m_arity;

public:
    VoidFunction(VoidFunctionCallback function, uint8_t arity)
	: m_function(function)
	, m_arity(arity)
    {
    }
    uint8_t arity() override { return m_arity; }
    RuntimeValue call(Args& args, RuntimeEnv& env) override
    {
	m_function(args, env);
	return RuntimeValue();
    }
};
class Function : public Callable {
private:
    FunctionCallback m_function;
    uint8_t m_arity;

public:
    Function(FunctionCallback function, uint8_t arity)
	: m_function(function)
	, m_arity(arity)
    {
    }
    uint8_t arity() override { return m_arity; }
    RuntimeValue call(Args& args, RuntimeEnv& env) override
    {
	return m_function(args, env);
    }
};
}
