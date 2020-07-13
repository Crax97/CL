#include "environment.hpp"
#include "commons.hpp"
#include "exceptions.hpp"

namespace Calculator {

class NotBoundException : public CLException {
public:
    NotBoundException(const std::string& unbound_name)
	: CLException(unbound_name + " is not bound")
    {
    }
};

StackedEnvironment::StackedEnvironment()
{
    m_scopes.push_back(Scope());
}

void StackedEnvironment::bind_in_current_scope(const std::string& name, RuntimeValue val)
{
    auto& scope = m_scopes.back();
    scope[name] = val;
}

RuntimeValue& StackedEnvironment::get_from_current(const std::string& name)
{
    for (auto scope = m_scopes.rbegin(); scope != m_scopes.rend(); scope++) {
	if (scope->find(name) != scope->end()) {
	    return scope->at(name);
	}
    }
    throw NotBoundException(name);
}

void StackedEnvironment::scope_in()
{
    m_scopes.push_back(Scope());
}
void StackedEnvironment::scope_out()
{
    m_scopes.pop_back();
}

void StackedEnvironment::assign(const std::string& name, RuntimeValue val)
{
    bind_in_current_scope(name, val);
}
RuntimeValue& StackedEnvironment::get(const std::string& name)
{
    return get_from_current(name);
}
}
