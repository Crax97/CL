#pragma once

#include "commons.hpp"
#include "exceptions.hpp"

#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace CL {

constexpr uint8_t VAR_ARGS = 0xFF;

class Stringable {
public:
	virtual std::string to_string() const noexcept = 0;
	virtual std::string string_repr() const noexcept {
		return to_string();
	};
};

class Indexable : public Stringable {
public:
	virtual void set(const RuntimeValue &, RuntimeValue v) = 0;
	virtual RuntimeValue &get(const RuntimeValue &) = 0;
	void set_named(const std::string &name, RuntimeValue v);
	RuntimeValue &get_named(const std::string &name);


};

class Callable : public Stringable  {
public:
	virtual std::optional<RuntimeValue> call(const Args &args) = 0;
	std::optional<RuntimeValue> call();
	virtual uint8_t arity() = 0;
	[[nodiscard]]
	virtual std::string to_string() const noexcept override {
		std::stringstream stream;
		stream << "LambdaStyleFunction @0x" << std::hex;
		auto addr = reinterpret_cast<uint64_t>(this);
		stream << addr;
		return stream.str();
	}
	[[nodiscard]]
	virtual std::string string_repr() const noexcept override { return to_string(); }
};

using RawValue = std::variant<std::monostate,
							  bool,
							  Number,
							  String,
							  IndexablePtr,
							  CallablePtr>;
class RuntimeValue {
private:
	struct rv_tag {};
	RawValue m_value;
	RuntimeValue(rv_tag, RawValue v)
		: m_value(std::move(v)) {

	}

public:
	[[nodiscard]]
	bool is_truthy() const noexcept;

	template<class T>
	[[nodiscard]]
	bool is() const noexcept { return std::holds_alternative<T>(m_value); }
	template<class T>
	[[nodiscard]]
	const T &as() const {
		if(is<T>()) {
			return std::get<T>(m_value);
		}
		throw RuntimeException(to_string() + " is not " + typeid(T).name());
	}

	template<class T>
	[[nodiscard]]
	const std::shared_ptr<T> as_object() const {
		if(is<IndexablePtr>()) {
			return std::dynamic_pointer_cast<T>(std::get<IndexablePtr>(m_value));
		}
		throw RuntimeException(to_string() + " is not an object of class" + typeid(T).name());
	}

	void negate();
	RuntimeValue operator+(const RuntimeValue &other);
	RuntimeValue operator-(const RuntimeValue &other) const;
	RuntimeValue operator*(const RuntimeValue &other) const;
	RuntimeValue operator/(const RuntimeValue &other) const;
	RuntimeValue modulo(const RuntimeValue &other) const;
	RuntimeValue to_power_of(const RuntimeValue &other) const;

	bool operator!=(const RuntimeValue &other) const;
	bool operator==(const RuntimeValue &other) const;
	bool operator<(const RuntimeValue &other) const;
	bool operator>(const RuntimeValue &other) const;
	bool operator<=(const RuntimeValue &other) const;
	bool operator>=(const RuntimeValue &other) const;

	explicit operator const std::string &() const {
		return as<std::string>();
	}
	explicit operator int() const {
		return static_cast<int>(as<Number>());
	}
	explicit operator Number() const {
		return as<Number>();
	}
	explicit operator bool() const {
		return static_cast<bool>(as<Number>());
	}

	void set_property(const RuntimeValue &name, RuntimeValue val) const {
		if(is<IndexablePtr>()) {
			auto ind = as<IndexablePtr>();
			ind->set(name, std::move(val));
		} else
			throw RuntimeException(to_string() + " is not indexable!");
	}

	[[nodiscard]]
	RuntimeValue &get_property(const RuntimeValue &name) const {
		if(is<IndexablePtr>()) {
			auto ind = as<IndexablePtr>();
			return ind->get(name);
		} else
			throw RuntimeException(to_string() + " is not indexable!");
	}

	void set_named(const std::string &name, RuntimeValue v) const {
		if(is<IndexablePtr>()) {
			auto ind = as<IndexablePtr>();
			ind->set_named(name, std::move(v));
		} else
			throw RuntimeException(to_string() + " is not indexable!");
	}

	[[nodiscard]]
	RuntimeValue get_named(const std::string &name) const {
		if(is<IndexablePtr>()) {
			auto ind = as<IndexablePtr>();
			return ind->get_named(name);
		} else
			throw RuntimeException(to_string() + " is not indexable!");
	}

	[[nodiscard]]
	const RawValue &raw_value() const noexcept;

	[[nodiscard]]
	std::string to_string() const noexcept;
	[[nodiscard]]
	std::string string_representation() const noexcept;

	explicit RuntimeValue(bool b) noexcept
		: m_value(b) {
	}
#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
	RuntimeValue(Number n) noexcept
		: m_value(n) {
	}
	RuntimeValue(String s) noexcept
		: m_value(s) {
	}
	RuntimeValue(CallablePtr c) noexcept
		: m_value(c) {
	}
	RuntimeValue(IndexablePtr p) noexcept
		: m_value(p) {
	}
	RuntimeValue() noexcept
		: m_value(std::monostate()) {
	}

	static RuntimeValue make_from_raw_value(RawValue value) {
		return RuntimeValue(rv_tag{}, std::move(value));
	}

#pragma clang diagnostic pop
};

using Dict = std::unordered_map<RawValue, RuntimeValue>;
using Lis = std::vector<RuntimeValue>;
class List : public Indexable {
private:
	Lis m_list;
	Dict m_functions;

public:
	List();
	void set(const RuntimeValue &s, RuntimeValue v) override {
		auto n = static_cast<size_t>(s.as<Number>());
		if(n < m_list.size()) {
			m_list[n] = v;
		} else {
			throw RuntimeException("Tried indexing outside this list's range");
		}
	}

	void append(const RuntimeValue &s) {
		m_list.push_back(s);
	}

	RuntimeValue &get(const RuntimeValue &s) override {
		if(!s.is<Number>()) {
			if(m_functions.find(s.raw_value()) == m_functions.end()) {
				throw RuntimeException(s.to_string() + " is not bound. ");
			}
			return m_functions.at(s.raw_value());
		}
		auto n = static_cast<size_t>(s.as<Number>());
		if(n < m_list.size()) {
			return m_list[n];
		} else {
			throw RuntimeException("Tried indexing outside this list's range");
		}
	}
	std::string to_string() const noexcept override {
		std::stringstream stream;
		stream << "[";
		for (const auto &v : m_list) {
			stream << v.to_string() << ", ";
		}
		stream << "]";
		return stream.str();
	}
	std::string string_repr() const noexcept override {
		return "list " + to_string();
	}
};

class Dictionary : public Indexable {
private:
	Dict m_map;

public:
	Dictionary();
	void set(const RuntimeValue &s, RuntimeValue v) override {
		m_map[s.raw_value()] = v;
	}
	RuntimeValue &get(const RuntimeValue &s) override {
		if(m_map.find(s.raw_value()) != m_map.end()) {
			return m_map.at(s.raw_value());
		}
		throw RuntimeException(s.to_string() + " not bound in dictionary\n");
	}
	std::string to_string() const noexcept override;
	std::string string_repr() const noexcept override;
};

class Module : public Indexable {

private:
	RuntimeEnvPtr m_env;

public:
	explicit Module(RuntimeEnvPtr env)
		: m_env(std::move(env)) {
	}

	RuntimeValue &get(const RuntimeValue &what) override;
	void set(const RuntimeValue &,
			 RuntimeValue) override {
		throw RuntimeException("Modules aren't externally modifiable.");
	}
	std::string to_string() const noexcept override;
	std::string string_repr() const noexcept override;
};
} // namespace CL
