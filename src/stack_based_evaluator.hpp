#pragma once

#include "exceptions.hpp"
#include <stack>

namespace Calculator {
template <class T>
class StackMachine {
private:
    std::stack<T> m_stack;

protected:
    void push(T el) noexcept { m_stack.push(el); }
    T& peek() { return m_stack.top(); }
    T pop()
    {
	if (!m_stack.size() > 0) {
	    throw RuntimeException("Tried popping on an empty stack");
	}
	auto t = m_stack.top();
	m_stack.pop();
	return t;
    }
};
}
