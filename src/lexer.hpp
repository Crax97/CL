#pragma once

#include "commons.hpp"

#include <istream>
#include <list>
#include <queue>
#include <sstream>
#include <string>

namespace CL {
class Token;
class Lexer {

private:
    std::istream& m_source;
    std::string m_current_source_line;

    std::queue<Token> m_parsed_tokens;
    uint64_t m_current_column { 1 };
    uint64_t m_current_line { 1 };
    bool m_done_lexing = false;


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
    explicit Lexer(std::istream& stream)
	: m_source(stream)
    {
	update_line_view();
    }

    Token next();
    Token& peek();

    [[maybe_unused]] [[maybe_unused]]
    void lex_all();
    [[nodiscard]]
    bool is_at_end() const noexcept;

    [[maybe_unused]] [[nodiscard]]
    [[maybe_unused]]
    bool has_tokens() const noexcept;
};
} // namespace CL
