#pragma once

#include "commons.hpp"
#include "value.hpp"
#include <unordered_map>
#include <vector>

namespace Calculator {
template <class T>
class Env {
public:
    virtual void scope_in() = 0;
    virtual void scope_out() = 0;
    virtual void assign(const std::string&, T) = 0;
    virtual T& get(const std::string&) = 0;
};

class StackedEnvironment : public Env<RuntimeValuePtr> {
private:
    using Scope = std::unordered_map<std::string, RuntimeValuePtr>;
    std::vector<Scope> m_scopes;
    void bind_in_current_scope(const std::string&, RuntimeValuePtr);
    RuntimeValuePtr& get_from_current(const std::string&);

public:
    explicit StackedEnvironment();
    void scope_in() override;
    void scope_out() override;
    void assign(const std::string&, RuntimeValuePtr) override;
    RuntimeValuePtr& get(const std::string&) override;
};
}
