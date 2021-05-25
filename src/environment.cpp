#include "environment.hpp"
#include "commons.hpp"
#include "exceptions.hpp"
#include <sstream>

namespace CL {

class NotBoundException : public CLException {
public:
	explicit NotBoundException(const std::string &unbound_name)
		: CLException(unbound_name + " is not bound") {
	}
};

RuntimeValue &StackedEnvironment::get(const std::string &name) {
	if(m_scope.find(name) != m_scope.end()) {
		return m_scope.at(name);
	}
	if(m_parent) {
		return m_parent->get(name);
	}
	throw NotBoundException(name);
}

void StackedEnvironment::assign(const std::string &name,
								RuntimeValue val,
								bool is_const) {
	auto current = shared_from_this();
	while (current != nullptr) {
		if(current->is_bound(name)) {
			current->bind(name, val, is_const);
			return;
		}
		current = current->m_parent;
	}
	bind(name, val, is_const);
}

void StackedEnvironment::bind(const std::string &name,
							  RuntimeValue val,
							  bool is_const) {

	if(m_consts.find(name) != m_consts.end()) {
		throw RuntimeException(name + " is const.");
	}
	m_scope[name] = val;
	if(is_const)
		m_consts.insert(name);
}

bool StackedEnvironment::is_bound(const std::string &name) {
	return m_scope.find(name) != m_scope.end();
}

std::string StackedEnvironment::to_string() const noexcept {
	std::stringstream stream;
	stream << "{\n";
	for (const auto &pair : m_scope) {
		stream << "\t" << pair.first << " : " << pair.second.to_string()
			   << "\n";
	}
	stream << "}";

	return stream.str();
}

}
