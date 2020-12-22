#include "lexer.hpp"
#include "commons.hpp"
#include "exceptions.hpp"
#include "tokens.hpp"

#include <cstdio>
#include <map>
#include <sstream>
#include <string>
#include <string_view>

namespace CL {
class LexerException : public CLException {
private:
	static std::string generate_nice_error(std::string_view error_message,
										   std::string_view source_line,
										   uint16_t line,
										   uint16_t column) {
		std::stringstream stream;
		stream << "Lexing error " << error_message << " at "
			   << std::to_string(line) << ":" << std::to_string(column);
		stream << "\n" << source_line << "\n";
		return stream.str();
	}

public:
	LexerException(std::string_view error_message,
				   std::string_view current_line,
				   uint64_t line,
				   uint64_t column)
		: CLException(generate_nice_error(error_message,
										  current_line,
										  line,
										  column)) {
	}
};

static std::map<std::string, TokenType>
	token_map = { // NOLINT(cert-err58-cpp)
	{"if", TokenType::If},
	{"else", TokenType::Else},
	{"while", TokenType::While},
	{"for", TokenType::For},
	{"in", TokenType::In},
	{"return", TokenType::Return},
	{"continue", TokenType::Continue},
	{"break", TokenType::Break},
	{"and", TokenType::And},
	{"or", TokenType::Or},
	{"function", TokenType::Fun},
    {"expose", TokenType::Expose},
	{"module", TokenType::Module},
	{"dict", TokenType::Dict},
	{"list", TokenType::List},
};

constexpr static std::string_view IGNORE_CHARS = "\n\t ";

char Lexer::get_next() {
	char c = m_source.get();
	m_current_column++;
	if(c == '\n') {
		m_current_line++;
		m_current_column = 0;
		update_line_view();
	}
	if (c == -1) {
	    m_done_lexing = true;
	}
	return c;
}

char Lexer::peekc() { return m_source.peek(); }
void Lexer::prev() {
	m_current_column--;
	if(m_source.peek() == '\n') {
		m_current_line--;
	}
	m_source.unget();
}

void Lexer::update_line_view() {
	m_current_source_line = "";
	auto pos = m_source.tellg();
	std::getline(m_source, m_current_source_line);
	m_source.seekg(pos);
}

template<class... T>
Token Lexer::make_token(uint64_t column,
						uint64_t line,
						const std::string &source,
						T... params) {
	return Token(params..., column, line, source);
}

Token Lexer::check_for_alternative(char expected, TokenType first,
								   TokenType second) {
	if(peekc() == expected) {
		get_next();
		return make_token(m_current_column - 1,
						  m_current_line,
						  m_current_source_line,
						  second);
	}
	return make_token(m_current_column,
					  m_current_line,
					  m_current_source_line,
					  first);
}
Token Lexer::parse_string(char delim) {
	auto line = m_current_line;
	auto column = m_current_column;
	String s;
	char c = get_next();
	while (c != delim) {
		if(c == '\\') {
			char selector = get_next();
			switch (selector) {
				case 'n': c = '\n';
					break;
				case 'b': c = '\b';
					break;
				case 'a': c = '\a';
					break;
				case 't': c = '\t';
					break;
				case 'f': c = '\f';
					break;
				case 'r': c = '\r';
					break;
				case 'v': c = '\v';
					break;
				case '"': c = '"';
					break;
				case '\'': c = '\'';
					break;
				case '\\': c = '\\';
					break;
				case 'x':
				case 'u':
				case 'U': TODO();
				default: s += c;
					c = selector;
					break;
			}
		}
		s += c;
		c = get_next();
		if(is_at_end()) {
			throw LexerException("Unexpected EOF while parsing string!",
								 m_current_source_line,
								 m_current_line,
								 m_current_column);
		}
	}
	return make_token(column,
					  line,
					  m_current_source_line,
					  TokenType::String,
					  s);
}

Token Lexer::parse_number() {
	auto line = m_current_line;
	auto column = m_current_column;
	bool met_dot = false;
	std::string number_string;
	char ch = get_next();
	while ((isdigit(ch) || ch == '.') && !is_at_end()) {
		number_string += ch;
		ch = get_next();
		if(ch == '.') {
			if(!met_dot) {
				met_dot = true;
			} else {
				throw LexerException("Invalid numeric literal!",
									 m_current_source_line,
									 line,
									 column);
			}
		}
	}
	prev();
	Number n = std::stod(number_string);
	return make_token(column, line, m_current_source_line, n);
}

Token Lexer::parse_keyword() {
	auto line = m_current_line;
	auto column = m_current_column;
	std::string token_string;
	auto ch = get_next();
	while ((isalpha(ch) || ch == '_' || isdigit(ch) || ch == ':')
		&& !is_at_end()) {
		token_string += ch;
		ch = get_next();
	}

	prev();

	auto it = token_map.find(token_string);
	if(it != token_map.end()) {
		return make_token(column, line, m_current_source_line, it->second);
	} else {
		return make_token(column,
						  line,
						  m_current_source_line,
						  TokenType::Identifier,
						  token_string);
	}
}

void Lexer::ignore_comment() {
	char ch = get_next();
	while (ch != '\n' && !m_done_lexing && ch != EOF) {
		ch = get_next();
	}
}

Token Lexer::try_lex_one() {
	char ch;
	do {
		ch = get_next();
		while (ch == '#') {
			ignore_comment();
			ch = get_next();
		}
	} while (IGNORE_CHARS.find(ch) != std::string::npos && !m_done_lexing);

	if(ch == EOF) {
		m_done_lexing = true;
		return make_token(m_current_column,
						  m_current_line,
						  m_current_source_line,
						  TokenType::Eof);
	}

	auto token_line = m_current_line;
	auto token_column = m_current_column;
	switch (ch) // NOLINT(hicpp-multiway-paths-covered)
	{
		case '+':
			return make_token(m_current_column,
							  m_current_line,
							  m_current_source_line,
							  TokenType::Plus);
		case '-':
			return check_for_alternative('>',
										 TokenType::Minus,
										 TokenType::Arrow);
		case '*':
			return make_token(m_current_column,
							  m_current_line,
							  m_current_source_line,
							  TokenType::Star);
		case '/':
			return make_token(m_current_column,
							  m_current_line,
							  m_current_source_line,
							  TokenType::Slash);
		case '%':
			return make_token(m_current_column,
							  m_current_line,
							  m_current_source_line,
							  TokenType::Percent);
		case '.':
			return make_token(m_current_column,
							  m_current_line,
							  m_current_source_line,
							  TokenType::Dot);
		case ',':
			return make_token(m_current_column,
							  m_current_line,
							  m_current_source_line,
							  TokenType::Comma);
		case '(':
			return make_token(m_current_column,
							  m_current_line,
							  m_current_source_line,
							  TokenType::Left_Brace);
		case ')':
			return make_token(m_current_column,
							  m_current_line,
							  m_current_source_line,
							  TokenType::Right_Brace);
		case '{':
			return make_token(m_current_column,
							  m_current_line,
							  m_current_source_line,
							  TokenType::Left_Curly_Brace);
		case '}':
			return make_token(m_current_column,
							  m_current_line,
							  m_current_source_line,
							  TokenType::Right_Curly_Brace);
		case '[':
			return make_token(m_current_column,
							  m_current_line,
							  m_current_source_line,
							  TokenType::Left_Square_Brace);
		case ']':
			return make_token(m_current_column,
							  m_current_line,
							  m_current_source_line,
							  TokenType::Right_Square_Brace);
		case ':':
			return make_token(m_current_column,
							  m_current_line,
							  m_current_source_line,
							  TokenType::Double_Dots);
		case ';':
			return make_token(m_current_column,
							  m_current_line,
							  m_current_source_line,
							  TokenType::Point_Comma);
		case '=':
			return check_for_alternative('=',
										 TokenType::Assign,
										 TokenType::Equals);
		case '!':
			return check_for_alternative('=',
										 TokenType::Not,
										 TokenType::Not_Equals);
		case '<':
			return check_for_alternative('=',
										 TokenType::Less,
										 TokenType::Less_Or_Equals);
		case '>':
			return check_for_alternative('=',
										 TokenType::Greater,
										 TokenType::Greater_Or_Equals);
		case '^':
			return make_token(m_current_column,
							  m_current_line,
							  m_current_source_line,
							  TokenType::Xor);
	}

	if(ch == '"' || ch == '\'') {
		return parse_string(ch);
	}
	if(isdigit(ch)) {
		prev();
		return parse_number();
	}
	if(isalpha(ch) || ch == '_') {
		prev();
		return parse_keyword();
	}

	// FAILURE IDENTIFYING TOKEN
	auto unknown_token = parse_keyword();
	throw LexerException("Unknown token: " + unknown_token.to_string(),
						 m_current_source_line,
						 token_line,
						 token_column);
}

Token Lexer::next() {

	auto token = peek();
	m_parsed_tokens.pop();
	return token;
}

Token &Lexer::peek() {
	if(m_parsed_tokens.empty()) {
		m_parsed_tokens.push(try_lex_one());
	}
	return m_parsed_tokens.back();
}

[[maybe_unused]] void Lexer::lex_all() {
	while (!is_at_end()) {
		m_parsed_tokens.push(try_lex_one());
	}
}

[[maybe_unused]] bool Lexer::has_tokens() const noexcept {
	return !m_parsed_tokens.empty() || !m_done_lexing;
}

bool Lexer::is_at_end() const noexcept {
	return m_done_lexing;
}
} // namespace CL
