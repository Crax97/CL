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

class Indexable {
public:
    virtual void set(const RuntimeValue&, RuntimeValue v) = 0;
    virtual RuntimeValue& get(const RuntimeValue&) = 0;

    virtual std::string to_string() = 0;
    virtual std::string string_repr() = 0;
};

class Callable {
public:
    virtual std::optional<RuntimeValue> call(const Args& args = Args()) = 0;
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

using RawValue = std::variant<std::monostate, Number, String, IndexablePtr, CallablePtr>;

class RuntimeValue {
private:
    RawValue m_value;
    bool m_constant { false };
    Dict m_map;

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

    void set_property(const RuntimeValue& name, RuntimeValue val)
    {
	if (is<IndexablePtr>()) {
	    auto ind = as<IndexablePtr>();
	    ind->set(name, val);
	} else
	    throw RuntimeException(to_string() + " is not indexable!");
    }
    RuntimeValue& get_property(const RuntimeValue& name)
    {
	if (is<IndexablePtr>()) {
	    auto ind = as<IndexablePtr>();
	    return ind->get(name);
	} else
	    throw RuntimeException(to_string() + " is not indexable!");
    }

    void set_named(const std::string& name, RuntimeValue v)
    {
	set_property(RuntimeValue(name), v);
    }
    RuntimeValue get_named(const std::string& name)
    {
	return get_property(RuntimeValue(name));
    }

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
    RuntimeValue(IndexablePtr p) noexcept
	: m_value(p)
    {
    }
    RuntimeValue() noexcept
	: m_value(std::monostate())
    {
    }
};

class Dictionary : public Indexable {
private:
    Dict m_map;

public:
    void set(const RuntimeValue& s, RuntimeValue v) override { m_map[s] = v; }
    RuntimeValue& get(const RuntimeValue& s) override { return m_map[s.as<String>()]; }
    virtual std::string to_string() override;
    virtual std::string string_repr() override;
};

class Module : public Indexable {

private:
    RuntimeEnvPtr m_env;

public:
    Module(RuntimeEnvPtr env)
	: m_env(env)
    {
    }

    const RuntimeEnvPtr get_env() const noexcept { return m_env; }
    RuntimeValue& get(const RuntimeValue& what) override;
    void set(const RuntimeValue&, RuntimeValue) override { throw RuntimeException("Modules aren't externally modifiable."); }
    virtual std::string to_string() override;
    virtual std::string string_repr() override;
};
} // namespace CL
