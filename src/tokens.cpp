#include "tokens.hpp"
#include "commons.hpp"

#include <optional>
#include <string>
#include <utility>

template <typename... Ts>
struct make_overload : Ts... {
    using Ts::operator()...;
};
template <typename... Ts>
make_overload(Ts...) -> make_overload<Ts...>;

template <typename Variant, typename... Alternatives>
decltype(auto) visit_variant(Variant&& variant,
    Alternatives&&... alternatives)
{
    return std::visit(make_overload { std::forward<Alternatives>(alternatives)... },
	std::forward<Variant>(variant));
}

namespace CL {

std::string token_type_to_string(TokenType type) noexcept
{
    switch (type) {
    case TokenType::Eof:
	return "Eof";
    case TokenType::Newline:
	return "Newline";
    case TokenType::Number:
	return "Number";
    case TokenType::Plus:
	return "+";
    case TokenType::Minus:
	return "-";
    case TokenType::Star:
	return "*";
    case TokenType::Slash:
	return "/";
    case TokenType::Percent:
	return "%";
    case TokenType::Equals:
	return "==";
    case TokenType::Dot:
	return ".";
    case TokenType::Comma:
	return ",";
    case TokenType::Not_Equals:
	return "!=";
    case TokenType::Left_Brace:
	return "(";
    case TokenType::Left_Square_Brace:
	return "[";
    case TokenType::Left_Curly_Brace:
	return "{";
    case TokenType::Right_Brace:
	return ")";
    case TokenType::Right_Square_Brace:
	return "]";
    case TokenType::Right_Curly_Brace:
	return "}";
    case TokenType::Less:
	return "<";
    case TokenType::Greater:
	return ">";
    case TokenType::Left_Shift:
	return "<<";
    case TokenType::Right_Shift:
	return ">>";
    case TokenType::Ampersand:
	return "&";
    case TokenType::Pipe:
	return "|";
    case TokenType::And:
	return "and";
    case TokenType::Or:
	return "or";
    case TokenType::Not:
	return "not";
    case TokenType::Xor:
	return "xor";
    case TokenType::Assign:
	return "=";
    case TokenType::Let:
	return "let";
    case TokenType::Fun:
	return "fun";
    case TokenType::Global:
	return "global";
    case TokenType::Identifier:
	return "id";
    case TokenType::If:
	return "if";
    case TokenType::Else:
	return "else";
    case TokenType::While:
	return "while";
    case TokenType::For:
	return "for";
    case TokenType::Return:
	return "return";
    case TokenType::Continue:
	return "continue";
    case TokenType::Break:
	return "break";
    case TokenType::Self:
	return "self";
    case TokenType::Double_Dots:
	return ":";
    case TokenType::Arrow:
	return "->";
    case TokenType::Dict:
	return "dict";
    case TokenType::List:
	return "list";
    default:
	return "TokenType not stringified " + std::to_string((uint8_t)type);
    }
}

Token::Token(TokenType type, CarriedValue value, uint16_t column, uint16_t line,
    std::string_view source_line) noexcept
    : m_type(type)
    , m_value(std::move(std::move(value)))
    , m_line(line)
    , m_column(column)
    , m_source_line(source_line)
{
}

Token::Token(Number number, uint16_t column, uint16_t line,
    std::string_view source_line) noexcept
    : Token(TokenType::Number, number, column, line, source_line)
{
}

Token::Token(TokenType type, uint16_t column, uint16_t line,
    std::string_view source_line) noexcept
    : m_type(type)
    , m_line(line)
    , m_column(column)
    , m_source_line(source_line)
    , m_value(std::nullopt)
{
}
Token::Token(TokenType type, std::string value, uint16_t column, uint16_t line,
    std::string_view source_line) noexcept
    : m_type(type)
    , m_value(value)
    , m_line(line)
    , m_column(column)
    , m_source_line(source_line)
{
}
std::string Token::to_string() const noexcept
{
    auto str = std::string("");
    str.append(token_type_to_string(m_type));
    if (m_value.has_value()) {
	str.append(" with value ");
	visit_variant(
	    m_value.value(),
	    [&str](Number i) { str.append(std::to_string(i)); },
	    [&str](const std::string& id) { str.append(id); });
    }
    return str;
}
} // namespace CL
