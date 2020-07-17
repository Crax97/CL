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
    : m_parent(nullptr)
{
}

void StackedEnvironment::bind_in_current_scope(const std::string& name, RuntimeValuePtr val)
{
    m_scope[name] = val;
}

RuntimeValuePtr& StackedEnvironment::get_from_current(const std::string& name)
{
    if (m_scope.find(name) != m_scope.end()) {
	return m_scope.at(name);
    }
    if (m_parent) {
	return m_parent->get(name);
    }
    throw NotBoundException(name);
}

void StackedEnvironment::assign(const std::string& name, RuntimeValuePtr val, bool is_const)
{
    if (m_consts.find(name) != m_consts.end()) {
	throw RuntimeException("Cannot assign to " + name + ": it is constant.");
    } else if (is_const) {
	m_consts.insert(name);
    }
    bind_in_current_scope(name, val);
}

RuntimeValuePtr& StackedEnvironment::get(const std::string& name)
{
    return get_from_current(name);
}
}
