#pragma once

#include "commons.hpp"
#include "exceptions.hpp"

#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace Calculator {
class RuntimeValue;

constexpr uint8_t VAR_ARGS = 0xFF;

class Callable {
public:
    virtual RuntimeValue call(Args& args, Env<RuntimeValue>& env) = 0;
    virtual uint8_t arity() = 0;
    virtual std::string to_string() const noexcept
    {
	std::stringstream sstream;
	sstream << "Function @0x" << std::hex;
	uint64_t addr = reinterpret_cast<uint64_t>(this);
	sstream << addr;
	return sstream.str();
    }
    virtual std::string string_repr() const noexcept { return to_string(); }
};

using RawValue = std::variant<std::monostate, Number, String, CallablePtr>;

class RuntimeValue {
private:
    RawValue m_value;
    std::map<std::string, RuntimeValue> m_map;

public:
    Number as_number() const;
    CallablePtr as_callable() const;

    bool is_truthy() const noexcept;
    bool is_null() const noexcept;
    bool is_number() const noexcept;
    bool is_string() const noexcept { return std::holds_alternative<String>(m_value); }
    bool is_callable() const noexcept;
    template <class T>
    bool is() const noexcept { return std::holds_alternative<T>(m_value); }

    template <class T>
    T as() const
    {
	if (is<T>()) {
	    return std::get<T>(m_value);
	}
	throw RuntimeException(to_string() + " is not " + typeid(T).name());
    }
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

    void set_property(const std::string& name, RuntimeValue val) { m_map[name] = val; }
    RuntimeValue& get_property(const std::string& name) { return m_map[name]; }

    RawValue& raw_value();

    std::string to_string() const noexcept;
    std::string string_representation() const noexcept;

    RuntimeValue(Number n) noexcept
	: m_value(n)
    {
    }
    RuntimeValue(String s) noexcept
	: m_value(s)
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
