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

struct dict_tag {
};

class Module {

private:
    RuntimeEnvPtr m_env;

public:
    Module(RuntimeEnvPtr env)
	: m_env(env)
    {
    }

    RuntimeValue& get(const std::string& what);
};

class Callable {
public:
    virtual RuntimeValue call(Args& args) = 0;
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

using RawValue = std::variant<std::monostate, Number, String, dict_tag, Module, CallablePtr>;

class RuntimeValue {
private:
    RawValue m_value;
    bool m_constant { false };
    Dict m_map;
    RuntimeValue(dict_tag tag)
	: m_value(tag)
    {
    }

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

    template <class T>
    static RuntimeValuePtr make(T arg) { return std::make_shared<RuntimeValue>(arg); }
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

    void mark_constant(bool value = true) noexcept { m_constant = value; }
    void set_property(const RuntimeValue& name, RuntimeValue val)
    {
	if (m_constant) {
	    throw RuntimeException("Cannot modify this object, as it's marked constant");
	}
	m_map[name] = val;
    }
    void set_named(const std::string& name, RuntimeValue val) { set_property(RuntimeValue(name), val); }
    RuntimeValue& get_property(const RuntimeValue& name)
    {
	if (std::holds_alternative<Module>(m_value)) {
	    auto as_mod = as<Module>();
	    return as_mod.get(name.as<String>());
	}
	return m_map[name];
    }
    RuntimeValue& get_named(const std ::string& name)
    {
	if (std::holds_alternative<Module>(m_value)) {
	    auto as_mod = as<Module>();
	    return as_mod.get(name);
	}
	return m_map[RuntimeValue(name)];
    }

    RawValue& raw_value();

    std::string to_string() const noexcept;
    std::string string_representation() const noexcept;

    static RuntimeValue make_dict() { return RuntimeValue(dict_tag()); }

    RuntimeValue(Number n) noexcept
	: m_value(n)
    {
    }
    RuntimeValue(String s) noexcept
	: m_value(s)
    {
    }
    RuntimeValue(Dict s) noexcept
	: m_value(dict_tag())
	, m_map(s)
    {
    }
    RuntimeValue(CallablePtr c) noexcept
	: m_value(c)
    {
    }
    RuntimeValue(Module m) noexcept
	: m_value(m)
    {
    }
    RuntimeValue() noexcept
	: m_value(std::monostate())
    {
    }
};
} // namespace CL
