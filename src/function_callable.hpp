#pragma once

#include "commons.hpp"
#include "value.hpp"

#include <optional>
#include <sstream>

namespace CL {
class VoidFunction : public Callable {
private:
    VoidFunctionCallback m_function;
    uint8_t m_arity;

public:
    VoidFunction(VoidFunctionCallback function, uint8_t arity)
	: m_function(std::move(function))
	, m_arity(arity)
    {
    }
    uint8_t arity() override { return m_arity; }
    [[nodiscard]]
    std::optional<RuntimeValue> call(const Args& args) override
    {
	m_function(args);
	return std::nullopt;
    }
};
class Function : public Callable {
private:
    FunctionCallback m_function;
    uint8_t m_arity;

public:
    Function(FunctionCallback function, uint8_t arity)
	: m_function(std::move(function))
	, m_arity(arity)
    {
    }
    uint8_t arity() override { return m_arity; }
    std::optional<RuntimeValue> call(const Args& args) override
    {
	return m_function(args);
    }
};
}
