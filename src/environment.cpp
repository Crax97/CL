#include "environment.hpp"
#include "commons.hpp"
#include "exceptions.hpp"
#include <sstream>

namespace Calculator {

class NotBoundException : public CLException {
public:
    NotBoundException(const std::string& unbound_name)
	: CLException(unbound_name + " is not bound")
    {
    }
};

RuntimeValuePtr& StackedEnvironment::get(const std::string& name)
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
    m_scope[name] = val;
}

std::string StackedEnvironment::to_string() const noexcept
{
    std::stringstream stream;
    stream << "{\n";
    for (const auto& pair : m_scope) {
	stream << "\t" << pair.first << " : " << pair.second->to_string() << "\n";
    }
    stream << "}";

    return stream.str();
}

}
