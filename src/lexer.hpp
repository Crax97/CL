#pragma once

#include "commons.hpp"

#include <istream>
#include <list>
#include <queue>
#include <sstream>
#include <string>

namespace Calculator {
class Token;
class Lexer {

private:
    std::istream& m_source;
    std::string m_current_source_line;

    std::queue<Token> m_parsed_tokens;
    uint64_t m_current_column { 1 };
    uint64_t m_current_line { 1 };
    bool m_done_lexing = false;

    bool match(char next);

    Token parse_number();
    Token parse_string(char delimiter);
    Token parse_keyword();

    template <class... T>
    Token make_token(T... params);
    Token check_for_alternative(char expected, TokenType first, TokenType second);

    Token try_lex_one();
    char get_next();
    char peekc();
    void prev();
    void update_line_view();
    void ignore_comment();

public:
    Lexer(std::istream& stream)
	: m_source(stream)
    {
	update_line_view();
    }

    Token next();
    Token& peek();
    void lex_all();
    bool is_at_end() const noexcept;
    bool has_tokens() const noexcept;
};
} // namespace CL
