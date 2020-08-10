#pragma once

#include "commons.hpp"
#include "value.hpp"

#include <optional>
#include <sstream>
#include <utility>

namespace CL {
class VoidFunction : public Callable {
private:
    VoidFunctionCallback m_function;
    uint8_t m_arity;

public:
    VoidFunction(VoidFunctionCallback function, uint8_t arity)
	: m_function(std::move(function))
	, m_arity(arity)
    {
    }
    uint8_t arity() override { return m_arity; }
    [[nodiscard]]
    std::optional<RuntimeValue> call(const Args& args) override
    {
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
	: m_function(std::move(function))
	, m_arity(arity)
    {
    }
    uint8_t arity() override { return m_arity; }
    std::optional<RuntimeValue> call(const Args& args) override
    {
	return m_function(args);
    }
};

namespace Detail {
    template<class U>
    U ensure_is_convertible(const RuntimeValue& arg) {
        static_assert(std::is_convertible<U, RuntimeValue>::value);
        return (U)arg;
    }

}

template<typename R, typename... Ts>
class Function : public Callable {
public:
    using function_type = std::function<R(Ts...)>;
private:
    function_type m_fun;

    template<size_t... I>
    std::optional<RuntimeValue> call_helper(const Args& args, std::index_sequence<I...>) {
        if constexpr (std::is_same<R, void>::value) {
            m_fun(Detail::ensure_is_convertible<Ts>(args[I])...);
            return std::nullopt;
        } else if constexpr (std::is_same<R, std::optional<RuntimeValue>>::value) {
            return m_fun(Detail::ensure_is_convertible<Ts>(args[I])...);
        } else {
            auto ret = m_fun(Detail::ensure_is_convertible<Ts>(args[I])...);
            if constexpr (std::is_arithmetic<R>::value)
                return RuntimeValue::make_from_raw_value(RawValue((Number)ret));
            else
                return RuntimeValue::make_from_raw_value(ret);
        }
    }
public:
    explicit Function(function_type fun)
        : m_fun(fun) { }
    uint8_t arity() override {
        return sizeof...(Ts);
    }

    std::optional<RuntimeValue> call(const Args& args) override {
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
