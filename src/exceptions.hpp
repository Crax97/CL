#pragma once

#include <stdexcept>
#include <string>
#include <utility>

namespace Calculator {
class CLException : public std::runtime_error {
protected:
    std::string m_message;

public:
    explicit CLException(std::string message)
	: std::runtime_error("")
	, m_message(std::move(message))
    {
    }

    [[nodiscard]]
    std::string_view get_message() const noexcept { return m_message; }

    [[nodiscard]]
    const char* what() const noexcept override { return m_message.c_str(); }
};

class RuntimeException : public CLException {
public:
    explicit RuntimeException(std::string message)
	: CLException(std::move(message))
    {
    }
};

} // namespace CL
