#pragma once

#include "commons.hpp"
#include "value.hpp"
#include "helpers.h"

#include <optional>
#include <sstream>
#include <utility>
#include <type_traits>

namespace CL {
class VoidFunction : public Callable {
private:
	VoidFunctionCallback m_function;
	uint8_t m_arity;

public:
	VoidFunction(VoidFunctionCallback function, uint8_t arity)
		: m_function(std::move(function)), m_arity(arity) {
	}
	uint8_t arity() override { return m_arity; }
	[[nodiscard]]
	std::optional<RuntimeValue> call(const Args &args) override {
		m_function(args);
		return std::nullopt;
	}
};
class LambdaStyleFunction : public Callable {
private:
	FunctionCallback m_function;
	uint8_t m_arity;

public:
	LambdaStyleFunction(FunctionCallback function, uint8_t arity)
		: m_function(std::move(function)), m_arity(arity) {
	}
	uint8_t arity() override { return m_arity; }
	std::optional<RuntimeValue> call(const Args &args) override {
		return m_function(args);
	}
};

namespace Detail {
template<class T>
// Avoid name clashing with c++20 std::remove_cvref
struct remove_cvref_mine {
	typedef std::remove_cv_t<std::remove_reference_t<T>> type;
};

template<typename From, typename To, typename = void>
struct is_convertible_to : std::false_type {};

template<typename From, typename To>
struct is_convertible_to<From, To,
						 std::void_t<decltype(static_cast<To>(std::declval<From>()))>>
	: std::true_type {
};

template<class U>
U ensure_is_convertible(const RuntimeValue &arg) {
	static_assert(is_convertible_to<RuntimeValue,
									typename remove_cvref_mine<U>::type>::value);
	return static_cast<U>(arg);
}

}

template<typename R, typename... Ts>
class Function : public Callable {
public:
	using function_type = std::function<R(Ts...)>;
private:
	function_type m_fun;

	template<size_t... I>
	std::optional<RuntimeValue> call_helper(const Args &args,
											std::index_sequence<I...>) {
		if constexpr (std::is_same<R, void>::value) {
			m_fun(Detail::ensure_is_convertible<Ts>(args[I])...);
			return std::nullopt;
		} else if constexpr (std::is_same<R,
										  std::optional<RuntimeValue>>::value) {
			return m_fun(Detail::ensure_is_convertible<Ts>(args[I])...);
		} else {
			auto ret = m_fun(Detail::ensure_is_convertible<Ts>(args[I])...);
			if constexpr (std::is_arithmetic<R>::value)
				return RuntimeValue::make_from_raw_value(RawValue((Number) ret));
			else
				return ret;
		}
	}
public:
	explicit Function(function_type fun)
		: m_fun(fun) {}
	uint8_t arity() override {
		return sizeof...(Ts);
	}

	std::optional<RuntimeValue> call(const Args &args) override {
		return call_helper(args, std::make_index_sequence<sizeof...(Ts)>{});
	}
};

template<typename R, typename... Ts>
static CallablePtr make_function(R(*fun)(Ts...)) {
	return std::make_shared<Function<R, Ts...>>(fun);
}
template<typename R, typename... Ts>
static CallablePtr make_function(std::function<R(Ts...)> fun) {
	return std::make_shared<Function<R, Ts...>>(fun);
}
}
