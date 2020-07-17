#pragma once

#include "commons.hpp"
#include "value.hpp"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Calculator {
template <class T>
class Env {
public:
    virtual void assign(const std::string&, T, bool is_const = false) = 0;
    virtual T& get(const std::string&) = 0;
};

class StackedEnvironment : public Env<RuntimeValuePtr>, std::enable_shared_from_this<StackedEnvironment> {
private:
    using Scope = std::unordered_map<std::string, RuntimeValuePtr>;
    Scope m_scope;
    std::unordered_set<std::string> m_consts;
    RuntimeEnvPtr m_parent { nullptr };
    void bind_in_current_scope(const std::string&, RuntimeValuePtr);
    RuntimeValuePtr& get_from_current(const std::string&);

public:
    StackedEnvironment(RuntimeEnvPtr parent)
	: m_parent(parent)
    {
    }
    explicit StackedEnvironment();
    void assign(const std::string&, RuntimeValuePtr, bool is_const = false) override;
    RuntimeValuePtr& get(const std::string&) override;
};
}
