#pragma once

#include "exceptions.hpp"
#include <stack>

namespace CL {
template<class T>
class StackMachine {
protected:
	std::stack<T> m_stack;
	void push(T el) noexcept { m_stack.push(el); }
	T &peek() { return m_stack.top(); }
	T pop() {
		if(m_stack.size() == 0) {
			throw RuntimeException("Tried popping on an empty stack");
		}
		auto t = m_stack.top();
		m_stack.pop();
		return t;
	}
};
}
