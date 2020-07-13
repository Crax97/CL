#include "lexer.hpp"
#include "commons.hpp"
#include "exceptions.hpp"
#include "tokens.hpp"

#include <cstdlib>
#include <map>
#include <string>
#include <string_view>

namespace Calculator {
class LexerException : public CLException {
private:
    static std::string generate_nice_error(std::string_view error_message, std::string_view source_line, uint16_t line, uint16_t column)
    {
	auto str = std::string("On line " + std::to_string(line) + ":" + std::to_string(column) + "\n");
	str.append(source_line);
	str.push_back('\n');
	for (size_t i = 0; i < column; i++) {
	    str.push_back('-');
	}
	str.push_back('^');
	for (size_t i = column + 1; i < source_line.size(); i++) {
	    str.push_back('-');
	}
	str.push_back('\n');
	str.append(error_message);
	str.push_back('\n');
	return str;
    }

public:
    LexerException(std::string_view error_message, std::string_view current_line, uint64_t line, uint64_t column)
	: CLException(generate_nice_error(error_message, current_line, line, column))
    {
    }
};

static std::map<std::string, TokenType>
    token_map = {
	{ "if", TokenType::If },
	{ "else", TokenType::Else },
	{ "while", TokenType::While },
	{ "for", TokenType::For },
	{ "in", TokenType::In },
	{ "return", TokenType::Return },
	{ "and", TokenType::And },
	{ "or", TokenType::Or },
	{ "fun", TokenType::Fun },
    };

constexpr static std::string_view IGNORE_CHARS = "\n\t ";

Lexer::Lexer(std::string& source)
    : m_source(source)
    , m_current_line { 1 }
    , m_current_column { 1 }
    , m_current_character(m_source.cbegin())
{
    update_line_view();
}

char Lexer::advance_stream()
{
    auto ch = *m_current_character++;
    update_col_and_line(ch);
    return ch;
}
char Lexer::peek_character()
{
    return *m_current_character;
}
char Lexer::previous_character()
{
    m_current_character--;
    m_current_column--;
    if (peek_character() == '\n')
	m_current_line--;
    return peek_character();
}

void Lexer::update_col_and_line(char ch) noexcept
{
    m_current_column++;
    if (ch == '\n') {
	m_current_line++;
	m_current_column = 1;
	update_line_view();
    }
}

void Lexer::update_line_view()
{
    m_current_source_line = "";
    auto begin = std::distance(m_source.cbegin(), m_current_character);
    auto next_newline = m_current_source_line.find('\n', begin);
    if (next_newline == std::string_view::npos) {
	next_newline = std::distance(m_current_character, m_source.cend());
    }
    auto size = next_newline - begin;
    m_current_source_line = std::string_view(m_source.data() + begin, size);
}

template <class... T>
Token Lexer::make_token(T... params)
{
    return Token(params..., m_current_column, m_current_line, m_current_source_line);
}

Token Lexer::check_for_alternative(char expected, TokenType first,
    TokenType second)
{
    if (peek_character() == expected) {
	advance_stream();
	return make_token(second);
    }
    return make_token(first);
}

Token Lexer::parse_number()
{
    std::string number_string;
    char ch = advance_stream();
    while (isdigit(ch) || ch == '.') {
	number_string += ch;
	ch = advance_stream();
    }
    previous_character();
    Number n = std::stod(number_string);
    return make_token(n);
}

Token Lexer::parse_keyword()
{
    std::string token_string;
    auto ch = advance_stream();
    while (isalpha(ch) || ch == '_' || isdigit(ch) || ch == ':') {
	token_string += ch;
	ch = advance_stream();
    }

    previous_character();

    auto it = token_map.find(token_string);
    if (it != token_map.end()) {
	return make_token(it->second);
    } else {
	return make_token(TokenType::Identifier, token_string);
    }
}

void Lexer::ignore_comment()
{
    char ch = advance_stream();
    while (ch != '\n') {
	ch = advance_stream();
    }
}

Token Lexer::try_lex_one()
{
    if (m_current_character == m_source.end()) {
	if (m_done_lexing)
	    throw LexerException("Tried to lex on an empty stream!", m_current_source_line, m_current_column, m_current_line);
	m_done_lexing = true;
	return make_token(TokenType::Eof);
    }

    auto ch = ' ';

    do {
	ch = advance_stream();
    } while (IGNORE_CHARS.find(ch) != std::string::npos);

    auto token_column = m_current_column;
    auto token_line = m_current_line;

    if (ch == '#') {
	ignore_comment();
	ch = advance_stream();
    }

    switch (ch) {
    case '+':
	return make_token(TokenType::Plus);
    case '-':
	return check_for_alternative('>', TokenType::Minus, TokenType::Arrow);
    case '*':
	return make_token(TokenType::Star);
    case '/':
	return make_token(TokenType::Slash);
    case '%':
	return make_token(TokenType::Percent);
    case '.':
	return make_token(TokenType::Dot);
    case ',':
	return make_token(TokenType::Comma);
    case '(':
	return make_token(TokenType::Left_Brace);
    case ')':
	return make_token(TokenType::Right_Brace);
    case '{':
	return make_token(TokenType::Left_Curly_Brace);
    case '}':
	return make_token(TokenType::Right_Curly_Brace);
    case '[':
	return make_token(TokenType::Left_Square_Brace);
    case ']':
	return make_token(TokenType::Right_Square_Brace);
    case ':':
	return make_token(TokenType::Double_Dots);
    case ';':
	return make_token(TokenType::Point_Comma);
    case '=':
	return check_for_alternative('=', TokenType::Assign, TokenType::Equals);
    case '!':
	return check_for_alternative('=', TokenType::Not, TokenType::Not_Equals);
    case '<':
	return check_for_alternative('=', TokenType::Less, TokenType::Less_Or_Equals);
    case '>':
	return check_for_alternative('=', TokenType::Greater, TokenType::Greater_Or_Equals);
    case '^':
	return make_token(TokenType::Xor);
    }

    if (isdigit(ch)) {
	previous_character();
	return parse_number();
    }
    if (isalpha(ch) || ch == '_') {
	previous_character();
	return parse_keyword();
    }

    // FAILURE IDENTIFYING TOKEN
    auto unknown_token = parse_keyword();
    throw LexerException("Unknown token: " + unknown_token.to_string(), m_current_source_line, token_line, token_column);
}

Token Lexer::next()
{

    auto token = peek();
    m_parsed_tokens.pop();
    return token;
}

Token& Lexer::peek()
{
    if (m_parsed_tokens.empty()) {
	m_parsed_tokens.push(try_lex_one());
    }
    return m_parsed_tokens.back();
}

void Lexer::lex_all()
{
    while (!is_at_end()) {
	m_parsed_tokens.push(try_lex_one());
    }
}

bool Lexer::is_at_end() noexcept
{
    return m_done_lexing;
}
} // namespace CL
