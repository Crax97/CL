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

namespace std {
template <>
struct hash<Calculator::RuntimeValue> {
    const size_t operator()(const Calculator::RuntimeValue& m);
};
};

namespace Calculator {

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

using RawValue = std::variant<std::monostate, bool, Number, String, IndexablePtr, CallablePtr>;
class RuntimeValue {
private:
    RawValue m_value;
    bool m_constant { false };

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

    const RawValue& raw_value() const noexcept;

    std::string to_string() const noexcept;
    std::string string_representation() const noexcept;

    explicit RuntimeValue(bool b) noexcept
	: m_value(b)
    {
    }
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

using Dict = std::unordered_map<RawValue, RuntimeValue>;
using Lis = std::vector<RuntimeValue>;
class List : public Indexable {
private:
    Lis m_list;
    Dict m_funs;

public:
    List();
    void set(const RuntimeValue& s, RuntimeValue v) override
    {
	size_t n = static_cast<size_t>(s.as<Number>());
	if (n < m_list.size()) {
	    m_list[n] = v;
	} else {
	    throw RuntimeException("Tried indexing outside this list's range");
	}
    }

    void append(const RuntimeValue& s)
    {
	m_list.push_back(s);
    }

    RuntimeValue& get(const RuntimeValue& s) override
    {
	if (!s.is<Number>()) {
	    if (m_funs.find(s.raw_value()) == m_funs.end()) {
		throw RuntimeException(s.to_string() + " is not bound. ");
	    }
	    return m_funs.at(s.raw_value());
	}
	size_t n = static_cast<size_t>(s.as<Number>());
	if (n < m_list.size()) {
	    return m_list[n];
	} else {
	    throw RuntimeException("Tried indexing outside this list's range");
	}
    }
    virtual std::string to_string() override
    {
	std::stringstream stream;
	stream << "[";
	for (const auto& v : m_list) {
	    stream << v.to_string() << ", ";
	}
	stream << "]";
	return stream.str();
    }
    virtual std::string string_repr() override
    {
	return "list " + to_string();
    }
};

class Dictionary : public Indexable {
private:
    Dict m_map;

public:
    Dictionary();
    void set(const RuntimeValue& s, RuntimeValue v) override
    {
	m_map[s.raw_value()] = v;
    }
    RuntimeValue& get(const RuntimeValue& s) override
    {
	if (m_map.find(s.raw_value()) != m_map.end()) {
	    return m_map.at(s.raw_value());
	}
	throw RuntimeException(s.to_string() + " not bound in dictionary\n");
    }
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
