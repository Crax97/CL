#include "value.hpp"
#include "commons.hpp"
#include "environment.hpp"
#include "exceptions.hpp"
#include "function_callable.hpp"

#include <cmath>
#include <ios>
#include <memory>
#include <optional>
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
    RawValue operator()(bool b) { return !b; }
    RawValue operator()(const Number v) { return -v; }
    template <class T>
    RawValue operator()(const T& _v)
    {
	throw RuntimeException("Cannot negate this value");
    }
};

struct StringVisitor {
    std::string operator()(const std::monostate) { return "nool"; }
    std::string operator()(bool b) { return std::to_string(b); }
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
    std::string operator()(const std::monostate) { return "nool"; }
    std::string operator()(bool b) { return std::to_string(b); }
    std::string operator()(const Number v)
    {
	return std::to_string(v);
    }
    std::string operator()(const CallablePtr& call) { return call->string_repr(); }
    std::string operator()(const String& str) { return "\"" + str + "\""; }
    std::string operator()(const IndexablePtr& ptr) { return ptr->string_repr(); }
};

struct TruthinessVisitor {
    bool operator()(bool b) { return b; }
    bool operator()(const Number v) { return v == 1.0; }
    template <class T>
    bool operator()(const T& _v) { return false; }
};

struct EqualityOperator {
    bool operator()(bool b, bool l) { return b == l; }
    bool operator()(const Number v, const Number o) { return v == o; }
    bool operator()(const std::monostate, const std::monostate) { return true; }
    bool operator()(const CallablePtr& l, const CallablePtr& r) { return l == r; }
    bool operator()(const IndexablePtr& l, const IndexablePtr& r) { return l == r; }
    bool operator()(const String& l, const String& r) { return l == r; }
    template <class T, class V>
    bool operator()(const T& _t, const V& _v) { return false; }
};
struct NegatedEqualityOperator {
    bool operator()(bool b, bool l) { return b != l; }
    bool operator()(const Number v, const Number o) { return v != o; }
    bool operator()(const std::monostate, const std::monostate) { return false; }
    bool operator()(const CallablePtr& l, const CallablePtr& r) { return l != r; }
    bool operator()(const IndexablePtr& l, const IndexablePtr& r) { return l != r; }
    bool operator()(const String& l, const String& r) { return l != r; }
    template <class T, class V>
    bool operator()(const T& _t, const V& _v) { return false; }
};

struct LessThanOperator {
    bool operator()(const Number v, const Number o) { return v < o; }
    bool operator()(const std::monostate, const std::monostate) { return false; }
    bool operator()(const String& l, const String& r) { return l < r; }
    template <class T, class V>
    bool operator()(const T& _t, const V& _v)
    {
	return reinterpret_cast<uint64_t>(&_t) < reinterpret_cast<uint64_t>(&_v);
    }
};
struct GreaterThanOperator {
    bool operator()(const Number v, const Number o) { return v > o; }
    bool operator()(const std::monostate, const std::monostate) { return false; }
    bool operator()(const String& l, const String& r) { return l > r; }
    template <class T, class V>
    bool operator()(const T& _t, const V& _v) { return false; }
};
struct LessEqualsOperator {
    bool operator()(const Number v, const Number o) { return v <= o; }
    bool operator()(const std::monostate, const std::monostate) { return false; }
    bool operator()(const String& l, const String& r) { return l <= r; }
    template <class T, class V>
    bool operator()(const T& _t, const V& _v) { return false; }
};

struct GreaterEqualsOperator {
    bool operator()(const Number v, const Number o) { return v >= o; }
    bool operator()(const std::monostate, const std::monostate) { return false; }
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

bool RuntimeValue::is_truthy() const noexcept
{
    return std::visit(TruthinessVisitor {}, m_value);
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
RuntimeValue RuntimeValue::operator-(const RuntimeValue& other) const
{
    return this->as<Number>() - other.as<Number>();
}
RuntimeValue RuntimeValue::operator*(const RuntimeValue& other) const
{
    return this->as<Number>() * other.as<Number>();
}
RuntimeValue RuntimeValue::operator/(const RuntimeValue& other) const
{
    return this->as<Number>() / other.as<Number>();
}
RuntimeValue RuntimeValue::to_power_of(const RuntimeValue& other) const
{
    return pow(this->as<Number>(), other.as<Number>());
}
RuntimeValue RuntimeValue::modulo(const RuntimeValue& other) const
{
    return fmod(this->as<Number>(), other.as<Number>());
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
    return std::visit(GreaterEqualsOperator {}, m_value, other.m_value);
}

std::string RuntimeValue::to_string() const noexcept
{
    return std::visit(StringVisitor {}, m_value);
}

std::string RuntimeValue::string_representation() const noexcept
{
    return std::visit(StringRepresentationVisitor {}, m_value);
}

const RawValue& RuntimeValue::raw_value() const noexcept { return m_value; }
RuntimeValue& Module::get(const RuntimeValue& what)
{
    if (!what.is<String>()) {
	throw RuntimeException("Modules are only indexable by strings!");
    }
    return m_env->get(what.as<String>());
}

std::string Module::to_string() { return "Module " + addr_to_hex_str(*this); }
std::string Module::string_repr() { return "module " + m_env->to_string(); }

List::List()
{
    m_functions["find"] = RuntimeValue(std::make_shared<Function>(
	[this](const Args& args) {
	    auto it = std::find(m_list.begin(), m_list.end(), args[0]);
	    if (it == m_list.end())
		return static_cast<Number>(-1);
	    return static_cast<Number>(std::distance(m_list.begin(), it));
	},
	1));
    m_functions["contains"] = RuntimeValue(std::make_shared<Function>(
	[this](const Args& args) {
	    auto it = std::find(m_list.begin(), m_list.end(), args[0]);
	    return it != m_list.end();
	},
	1));
    m_functions["append"] = RuntimeValue(std::make_shared<VoidFunction>([this](const Args& args) {
	this->append(args[0]);
    },
                                                                        1));
}

Dictionary::Dictionary()
{
    m_map["contains"] = RuntimeValue(std::dynamic_pointer_cast<Callable>(std::make_shared<Function>(
	[this](const Args& args) {
	    return m_map.find(args[0].raw_value()) != m_map.end();
	},
	1)));
}
std::string Dictionary::to_string() { return "Dictionary " + addr_to_hex_str(*this); }
std::string Dictionary::string_repr()
{
    std::stringstream stream;
    stream << " {\n";
    for (const auto& pair : m_map) {
	stream << "\t" << std::visit(StringVisitor {}, pair.first) << " : " << pair.second.to_string() << "\n";
    }
    stream << "}";
    return "dict " + stream.str();
}

    std::optional<RuntimeValue> Callable::call() {
        return call(Args());
    }
} // namespace CL
