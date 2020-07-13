#pragma once

#include "commons.hpp"

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace Calculator {
class RuntimeValue;

class Callable {
public:
    virtual RuntimeValue call(Args& args, Env<RuntimeValue>& env) = 0;
    virtual uint8_t arity() = 0;
};

using RawValue = std::variant<std::monostate, Number, CallablePtr>;

class RuntimeValue {
private:
    RawValue m_value;

public:
    Number as_number() const;
    CallablePtr as_callable() const;

    bool is_truthy() const noexcept;
    bool is_null() const noexcept;
    bool is_number() const noexcept;
    bool is_callable() const noexcept;
    void negate();
    RuntimeValue operator+(const RuntimeValue& other);
    RuntimeValue operator-(const RuntimeValue& other);
    RuntimeValue operator*(const RuntimeValue& other);
    RuntimeValue operator/(const RuntimeValue& other);
    RuntimeValue modulo(const RuntimeValue& other);
    RuntimeValue to_power_of(const RuntimeValue& other);

    bool operator!=(const RuntimeValue& other) const;
    bool operator==(const RuntimeValue& other) const;
    bool operator<(const RuntimeValue& other) const;
    bool operator>(const RuntimeValue& other) const;
    bool operator<=(const RuntimeValue& other) const;
    bool operator>=(const RuntimeValue& other) const;

    RawValue& raw_value();

    std::string to_string() const noexcept;
    std::string string_representation() const noexcept;

    RuntimeValue(Number n) noexcept
	: m_value(n)
    {
    }
    RuntimeValue(CallablePtr c) noexcept
	: m_value(c)
    {
    }
    RuntimeValue() noexcept
	: m_value(std::monostate())
    {
    }
};
} // namespace CL
