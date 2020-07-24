#include "value.hpp"
#include "commons.hpp"
#include "environment.hpp"
#include "exceptions.hpp"

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
    std::string operator()(const Module& mod) { return "Module TODO"; }
    std::string operator()(const dict_tag& tag)
    {
	std::stringstream str;
	str << "Dict { \n";
#if 0
	for (const auto& couple : m_map) {
	    str << couple.first << " : " << couple.second << ",\n";
	}
#endif
	str << "}";
	return str.str();
    }

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
    std::string operator()(const dict_tag& s) { return "{}"; }
    std::string operator()(const CallablePtr& call) { return call->string_repr(); }
    std::string operator()(const String& str) { return "\"" + str + "\""; }
    std::string operator()(const Module& mod) { return "module {}\n"; }
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
    if (!std::holds_alternative<dict_tag>(m_value)) {
	return std::visit(StringVisitor {}, m_value);
    } else {
	return "Dict " + string_representation();
    }
}

std::string RuntimeValue::string_representation() const noexcept
{
    // Ew hacks
    if (!std::holds_alternative<dict_tag>(m_value)) {
	return std::visit(StringRepresentationVisitor {}, m_value);
    } else {
	std::stringstream sstr;
	sstr << "{\n";
	for (const auto& entries : m_map) {
	    sstr << "\t" << entries.first.to_string() << " : " << entries.second.to_string() << ",\n";
	}
	sstr << "}";
	return sstr.str();
    }
}

RawValue& RuntimeValue::raw_value() { return m_value; }
RuntimeValue& Module::get(const std::string& what) { return *m_env->get(what); }
} // namespace CL
