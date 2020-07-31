#include "lexer.hpp"
#include "commons.hpp"
#include "exceptions.hpp"
#include "tokens.hpp"

#include <cstdio>
#include <map>
#include <sstream>
#include <string>
#include <string_view>

namespace Calculator
{
	class LexerException : public CLException
	{
	private:
		static std::string generate_nice_error(std::string_view error_message, std::string_view source_line, uint16_t line, uint16_t column)
		{
			std::stringstream stream;
			stream << "Lexing error " << error_message;
			return stream.str();
		}

	public:
		LexerException(std::string_view error_message, std::string_view current_line, uint64_t line, uint64_t column)
			: CLException(generate_nice_error(error_message, current_line, line, column))
		{
		}
	};

	static std::map<std::string, TokenType>
		token_map = {
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
			{"fun", TokenType::Fun},
			{"module", TokenType::Module},
			{"dict", TokenType::Dict},
			{"list", TokenType::List},
	};

	constexpr static std::string_view IGNORE_CHARS = "\n\t ";

	char Lexer::get_next()
	{
		char c = m_source.get();
		m_current_column++;
		if (c == '\n')
		{
			m_current_line++;
			m_current_column = 1;
			update_line_view();
		}
		return c;
	}

	char Lexer::peekc() { return m_source.peek(); }
	void Lexer::prev()
	{
		m_current_column--;
		m_source.unget();
	}

	void Lexer::update_line_view()
	{
		m_current_source_line = "";
		auto pos = m_source.tellg();
		std::getline(m_source, m_current_source_line);
		m_source.seekg(pos);
	}

	template <class... T>
	Token Lexer::make_token(T... params)
	{
		return Token(params..., m_current_column, m_current_line, m_current_source_line);
	}

	Token Lexer::check_for_alternative(char expected, TokenType first,
									   TokenType second)
	{
		if (peekc() == expected)
		{
			m_source.get();
			return make_token(second);
		}
		return make_token(first);
	}
	Token Lexer::parse_string(char delim)
	{
		String s;
		char c = get_next();
		while (c != delim)
		{
			if (c == '\\')
			{
				char selector = get_next();
				switch (selector)
				{
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
                default:
				    s += c;
				    c = selector;
				    break;
				}
			}
			s += c;
			c = get_next();
			if (is_at_end())
			{
				throw LexerException("Unexpected EOF while parsing string!", m_current_source_line, m_current_line, m_current_column);
			}
		}
		return make_token(TokenType::String, s);
	}

	Token Lexer::parse_number()
	{
		std::string number_string;
		char ch = get_next();
		while ((isdigit(ch) || ch == '.') && !is_at_end())
		{
			number_string += ch;
			ch = get_next();
		}
		prev();
		Number n = std::stod(number_string);
		return make_token(n);
	}

	Token Lexer::parse_keyword()
	{
		std::string token_string;
		auto ch = get_next();
		while ((isalpha(ch) || ch == '_' || isdigit(ch) || ch == ':') && !is_at_end())
		{
			token_string += ch;
			ch = get_next();
		}

		prev();

		auto it = token_map.find(token_string);
		if (it != token_map.end())
		{
			return make_token(it->second);
		}
		else
		{
			return make_token(TokenType::Identifier, token_string);
		}
	}

	void Lexer::ignore_comment()
	{
		char ch = get_next();
		while (ch != '\n' && !m_done_lexing && ch != EOF)
		{
			ch = get_next();
		}
	}

	Token Lexer::try_lex_one()
	{
		char ch;
		do
		{
			ch = get_next();
			while (ch == '#')
			{
				ignore_comment();
				ch = get_next();
			}
		} while (IGNORE_CHARS.find(ch) != std::string::npos && !m_done_lexing);

		if (ch == EOF)
		{
			m_done_lexing = true;
			return make_token(TokenType::Eof);
		}

        auto token_line = m_current_line;
		auto token_column = m_current_column;
		switch (ch) // NOLINT(hicpp-multiway-paths-covered)
		{
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

		if (ch == '"' || ch == '\'')
		{
			return parse_string(ch);
		}
		if (isdigit(ch))
		{
			m_source.unget();
			return parse_number();
		}
		if (isalpha(ch) || ch == '_')
		{
			m_source.unget();
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

	Token &Lexer::peek()
	{
		if (m_parsed_tokens.empty())
		{
			m_parsed_tokens.push(try_lex_one());
		}
		return m_parsed_tokens.back();
	}

	void Lexer::lex_all()
	{
		while (!is_at_end())
		{
			m_parsed_tokens.push(try_lex_one());
		}
	}

	bool Lexer::has_tokens() const noexcept
	{
		return !m_parsed_tokens.empty() || !m_done_lexing;
	}

	bool Lexer::is_at_end() const noexcept
	{
		return m_done_lexing;
	}
} // namespace Calculator
