#pragma once

#include <stdexcept>
#include <string>

namespace Calculator {
class CLException : public std::runtime_error {
protected:
    std::string m_message;

public:
    CLException(std::string message)
	: std::runtime_error("")
	, m_message(message)
    {
    }

    std::string_view get_message() const noexcept { return m_message; }

    const char* what() const noexcept override { return m_message.c_str(); }
};

class RuntimeException : public CLException {
public:
    RuntimeException(std::string message)
	: CLException(message)
    {
    }
};

class CompilerException : public CLException {
public:
    CompilerException(std::string message)
	: CLException(message)
    {
    }
};
} // namespace CL
