#include "value.hpp"
#include "commons.hpp"
#include "environment.hpp"
#include "exceptions.hpp"
#include "function_callable.hpp"

#include <cmath>
#include <ios>
#include <sstream>
#include <variant>

std::string num_to_str_pretty_formatted(double n)
{
    std::string repr = std::to_string(n);
    auto last_index_of = repr.find_last_not_of('0');
    repr.erase(last_index_of, repr.size());
    if (repr[repr.size() - 1] == '.')
	repr.pop_back();
    return repr;
}

template <class T>
std::string addr_to_hex_str(const T& el)
{
    std::stringstream str;
    str << "@0x" << std::hex;
    str << reinterpret_cast<uint64_t>(&el);
    return str.str();
}

namespace Calculator {
struct NegateVisitor {
    RawValue operator()(const Number v) { return -v; }

    template <class T>
    RawValue operator()(const T& _v)
    {
	throw RuntimeException("Cannot negate this value");
    }
};

struct StringVisitor {
    std::string operator()(const std::monostate v) { return "nool"; }
    std::string operator()(const Number v) { return num_to_str_pretty_formatted(v); }
    std::string operator()(const Module& mod) { return "Module " + addr_to_hex_str(mod); }
    std::string operator()(const IndexablePtr& ptr) { return ptr->to_string(); }
    std::string operator()(const CallablePtr& call)
    {
	return call->to_string();
    }
    std::string operator()(const String& str)
    {
	return str;
    }
};

struct StringRepresentationVisitor {
    std::string operator()(const std::monostate v) { return "void"; }
    std::string operator()(const Number v)
    {
	return std::to_string(v);
    }
    std::string operator()(const CallablePtr& call) { return call->string_repr(); }
    std::string operator()(const String& str) { return "\"" + str + "\""; }
    std::string operator()(const IndexablePtr& ptr) { return ptr->string_repr(); }
};

struct TruthinessVisitor {
    bool operator()(const Number v) { return v == 1.0; }
    template <class T>
    bool operator()(const T& _v) { return false; }
};

struct EqualityOperator {
    bool operator()(const Number v, const Number o) { return v == o; }
    bool operator()(const std::monostate v, const std::monostate o) { return true; }
    bool operator()(const CallablePtr* l, const CallablePtr* r) { return l == r; }
    bool operator()(const String& l, const String& r) { return l == r; }
    template <class T, class V>
    bool operator()(const T& _t, const V& _v) { return false; }
};
struct NegatedEqualityOperator {
    bool operator()(const Number v, const Number o) { return v != o; }
    bool operator()(const std::monostate v, const std::monostate o) { return true; }
    bool operator()(const CallablePtr* l, const CallablePtr* r) { return l != r; }
    bool operator()(const String& l, const String& r) { return l != r; }
    template <class T, class V>
    bool operator()(const T& _t, const V& _v) { return false; }
};

struct LessThanOperator {
    bool operator()(const Number v, const Number o) { return v < o; }
    bool operator()(const std::monostate v, const std::monostate o) { return true; }
    bool operator()(const String& l, const String& r) { return l < r; }
    template <class T, class V>
    bool operator()(const T& _t, const V& _v) { return false; }
};
struct GreaterThanOperator {
    bool operator()(const Number v, const Number o) { return v > o; }
    bool operator()(const std::monostate v, const std::monostate o) { return false; }
    bool operator()(const String& l, const String& r) { return l > r; }
    template <class T, class V>
    bool operator()(const T& _t, const V& _v) { return false; }
};
struct LessEqualsOperator {
    bool operator()(const Number v, const Number o) { return v <= o; }
    bool operator()(const std::monostate v, const std::monostate o) { return false; }
    bool operator()(const String& l, const String& r) { return l <= r; }
    template <class T, class V>
    bool operator()(const T& _t, const V& _v) { return false; }
};

struct GreaterEqualsOperator {
    bool operator()(const Number v, const Number o) { return v >= o; }
    bool operator()(const std::monostate v, const std::monostate o) { return false; }
    bool operator()(const String& l, const String& r) { return l >= r; }
    template <class T, class V>
    bool operator()(const T& _t, const V& _v) { return false; }
};

struct SumOperator {
    RuntimeValue operator()(const Number n, const Number o) { return n + o; }
    template <class T>
    RuntimeValue operator()(const String& s, const T& o) { return s + StringVisitor()(o); }
    template <class T, class U>
    RuntimeValue operator()(const T& _t, const U& _u) { throw RuntimeException("Values cannot be summed."); }
};

Number
RuntimeValue::as_number() const
{
    if (std::holds_alternative<Number>(m_value)) {
	return std::get<Number>(m_value);
    } else {
	throw RuntimeException("This value is not a number! " + to_string());
    }
}

CallablePtr RuntimeValue::as_callable() const
{
    if (is_callable()) {
	return std::get<CallablePtr>(m_value);
    } else {
	throw RuntimeException(to_string() + " is not a Callable!");
    }
}

bool RuntimeValue::is_truthy() const noexcept
{
    return std::visit(TruthinessVisitor {}, m_value);
}

bool RuntimeValue::is_null() const noexcept
{
    return std::holds_alternative<std::monostate>(m_value);
}

bool RuntimeValue::is_callable() const noexcept
{
    return std::holds_alternative<CallablePtr>(m_value);
}

void RuntimeValue::negate()
{
    auto negated = std::visit(NegateVisitor {}, m_value);
    m_value = negated;
}

RuntimeValue RuntimeValue::operator+(const RuntimeValue& other)
{
    return std::visit(SumOperator {}, this->m_value, other.m_value);
}
RuntimeValue RuntimeValue::operator-(const RuntimeValue& other)
{
    return this->as_number() - other.as_number();
}
RuntimeValue RuntimeValue::operator*(const RuntimeValue& other)
{
    return this->as_number() * other.as_number();
}
RuntimeValue RuntimeValue::operator/(const RuntimeValue& other)
{
    return this->as_number() / other.as_number();
}
RuntimeValue RuntimeValue::to_power_of(const RuntimeValue& other)
{
    return pow(this->as_number(), other.as_number());
}
RuntimeValue RuntimeValue::modulo(const RuntimeValue& other)
{
    return fmod(this->as_number(), other.as_number());
}
bool RuntimeValue::operator!=(const RuntimeValue& other) const
{
    return std::visit(NegatedEqualityOperator {}, m_value, other.m_value);
}
bool RuntimeValue::operator==(const RuntimeValue& other) const
{
    return std::visit(EqualityOperator {}, m_value, other.m_value);
}
bool RuntimeValue::operator<(const RuntimeValue& other) const
{
    return std::visit(LessThanOperator {}, m_value, other.m_value);
}
bool RuntimeValue::operator>(const RuntimeValue& other) const
{
    return std::visit(GreaterThanOperator {}, m_value, other.m_value);
}
bool RuntimeValue::operator<=(const RuntimeValue& other) const
{
    return std::visit(LessEqualsOperator {}, m_value, other.m_value);
}
bool RuntimeValue::operator>=(const RuntimeValue& other) const
{
    auto truth_diocane = std::visit(GreaterEqualsOperator {}, m_value, other.m_value);
    return truth_diocane;
}

std::string RuntimeValue::to_string() const noexcept
{
    // Ew hacks
    return std::visit(StringVisitor {}, m_value);
}

std::string RuntimeValue::string_representation() const noexcept
{
    std::stringstream sstr;
    sstr << "{\n";
    for (const auto& entries : m_map) {
	sstr << "\t" << entries.first.to_string() << " : " << entries.second.to_string() << ",\n";
    }
    sstr << "}";
    return sstr.str();
}

RawValue& RuntimeValue::raw_value() { return m_value; }
RuntimeValue& Module::get(const RuntimeValue& what)
{
    if (!what.is<String>()) {
	throw RuntimeException("Modules are only indexable by strings!");
    }
    return m_env->get(what.as<String>());
}

std::string Module::to_string() { return "Module " + addr_to_hex_str(*this); }
std::string Module::string_repr() { return "module " + m_env->to_string(); }

Dictionary::Dictionary()
{
    m_map[RuntimeValue("contains")] = RuntimeValue(
	std::make_shared<Function>([this](const Args& args) {
	    return m_map.find(args[0]) != m_map.end();
	},
	    1));
}
std::string Dictionary::to_string() { return "Dictionary " + addr_to_hex_str(*this); }
std::string Dictionary::string_repr()
{
    std::stringstream stream;
    stream << " {\n";
    for (const auto& pair : m_map) {
	stream << "\t" << pair.first.to_string() << " : " << pair.second.to_string() << "\n";
    }
    stream << "}";
    return "dict " + stream.str();
}

} // namespace CL
