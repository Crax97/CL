#pragma once

#include "commons.hpp"

#include <istream>
#include <list>
#include <queue>
#include <string>

namespace Calculator {
class Token;
class Lexer {

private:
    std::string& m_source;
    std::string_view m_current_source_line;
    std::string::const_iterator m_current_character;

    std::queue<Token> m_parsed_tokens;
    uint64_t m_current_column;
    uint64_t m_current_line;
    bool m_done_lexing = false;

    bool match(char next);
    void update_col_and_line(char ch) noexcept;

    Token parse_number();
    Token parse_string(char delimiter);
    Token parse_keyword();

    template <class... T>
    Token make_token(T... params);
    Token check_for_alternative(char expected, TokenType first, TokenType second);

    Token try_lex_one();
    void update_line_view();
    void ignore_comment();

    char advance_stream();
    char peek_character();
    char previous_character();

public:
    Lexer(std::string& source);

    Token next();
    Token& peek();
    void lex_all();
    bool is_at_end() noexcept;
};
} // namespace CL
