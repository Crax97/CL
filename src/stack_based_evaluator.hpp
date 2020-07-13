#pragma once

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
	auto t = m_stack.top();
	m_stack.pop();
	return t;
    }
};
}
