#pragma once

#include "commons.hpp"

#include <optional>
#include <string_view>
#include <variant>
namespace Calculator {
class Token {
private:
    using CarriedValue = std::optional<std::variant<Number, std::string>>;
    TokenType m_type;
    CarriedValue m_value;
    std::string_view m_source_line;
    uint16_t m_line;
    uint16_t m_column;

public:
    Token(TokenType type, CarriedValue value, uint16_t column,
	uint16_t line, std::string_view source_line) noexcept;
    Token(Number number, uint16_t column, uint16_t line, std::string_view source_line) noexcept;
    Token(TokenType type, uint16_t column, uint16_t line, std::string_view source_line) noexcept;
    Token(TokenType type, std::string string, uint16_t columtn, uint16_t line, std::string_view source_line) noexcept;
    [[nodiscard]] TokenType get_type() const noexcept { return m_type; }
    [[nodiscard]] uint16_t get_line() const noexcept { return m_line; }
    [[nodiscard]] uint16_t get_column() const noexcept { return m_column; }
    [[nodiscard]] std::string_view get_source_line() const noexcept { return m_source_line; }

    [[nodiscard]] std::string to_string() const noexcept;
    template <class T>
    T get()
    {
	return std::get<T>(m_value.value());
    }
};
} // namespace Calculator
