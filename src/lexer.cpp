#include "lexer.hpp"
#include "commons.hpp"
#include "exceptions.hpp"
#include "tokens.hpp"

#include <cstdlib>
#include <map>
#include <sstream>
#include <string_view>

namespace Calculator {
class LexerException : public CLException {
private:
    static std::string generate_nice_error(std::string_view error_message, std::string_view source_line, uint16_t line, uint16_t column)
    {
	std::stringstream sstream;
	sstream << "Lexing error " << error_message;
	return sstream.str();
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
	{ "module", TokenType::Module },
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
    if (m_current_character == m_source.cend()) {
	m_done_lexing = true;
	return 0;
    }
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
    m_current_source_line = m_source.substr(begin, size);
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
Token Lexer::parse_string(char delim)
{
    String s;
    char c = advance_stream();
    while (c != delim) {
	if (c == '\\') {
	    char selector = advance_stream();
	    switch (selector) {
	    case 'n':
		c = '\n';
		break;
	    case 'b':
		c = '\b';
		break;
	    case 'a':
		c = '\a';
		break;
	    case 't':
		c = '\t';
		break;
	    case 'f':
		c = '\f';
		break;
	    case 'r':
		c = '\r';
		break;
	    case 'v':
		c = '\v';
		break;
	    case '"':
		c = '"';
		break;
	    case '\'':
		c = '\'';
		break;
	    case '\\':
		c = '\\';
		break;
	    case 'x':
	    case 'u':
	    case 'U':
		TODO();
		break;
	    }
	}
	s += c;
	c = advance_stream();
	if (is_at_end()) {
	    throw LexerException("Unexpected EOF while parsing string!", m_current_source_line, m_current_line, m_current_column);
	}
    }
    return make_token(TokenType::String, s);
}

Token Lexer::parse_number()
{
    std::string number_string;
    char ch = advance_stream();
    while ((isdigit(ch) || ch == '.') && !is_at_end()) {
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
    while ((isalpha(ch) || ch == '_' || isdigit(ch) || ch == ':') && !is_at_end()) {
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
    while (ch != '\n' && !m_done_lexing) {
	ch = advance_stream();
    }
}

Token Lexer::try_lex_one()
{
    auto ch = ' ';

    do {
	ch = advance_stream();
	while (ch == '#') {
	    ignore_comment();
	    ch = advance_stream();
	}
    } while (IGNORE_CHARS.find(ch) != std::string::npos && !m_done_lexing);

    if (m_done_lexing) {
	return make_token(TokenType::Eof);
    }
    auto token_column = m_current_column;
    auto token_line = m_current_line;

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

    if (ch == '"' || ch == '\"') {
	return parse_string(ch);
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

bool Lexer::has_tokens() const noexcept
{
    return m_parsed_tokens.size() > 0 || !m_done_lexing;
}

bool Lexer::is_at_end() noexcept
{
    return m_done_lexing;
}
} // namespace CL
