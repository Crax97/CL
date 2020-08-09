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
    template<class T, typename = void>
    struct is_callable
            : std::false_type {};

    template<class T>
    struct is_callable<T, std::void_t<decltype(std::declval<T>()())>>
            : std::true_type{};

    template<class U>
    U ensure_is_same(const RuntimeValue& arg) {
        if (!arg.is<U>()) {
            throw RuntimeException("Argument is not valid");
        }
        return arg.as<U>();
    }

}

template<typename R, typename... Ts>
class Function : public Callable {
public:
    using return_type = typename std::conditional<std::is_same<R, void>::value, void, std::optional<RuntimeValue>>::type;
    using function_type = std::function<return_type(Ts...)>;
private:
    function_type m_fun;

    template<size_t... I>
    std::optional<RuntimeValue> call_helper(const Args& args, std::index_sequence<I...>) {
        return m_fun(Detail::ensure_is_same<Ts>(args[I])...);
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

template<typename... Ts>
static CallablePtr make_function(std::optional<RuntimeValue>(*fun)(Ts...)) {
    return std::make_shared<Function<Ts...>>(std::move(fun));
}
template<typename Ret, typename... Ts>
static CallablePtr make_function(std::function<Ret(Ts...)> fun) {
    return std::make_shared<Function<Ret, Ts...>>(std::move(fun));
}
template<typename Ret, typename... Ts>
static CallablePtr make_function(Ret(*fun)(Ts...)) {
    return std::make_shared<Function<Ret, Ts...>>(std::move(fun));
}
}
