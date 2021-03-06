#pragma once

#include <deque>

#include "commons.hpp"
#include "lexer.hpp"
#include "nodes.hpp"
#include "tokens.hpp"

/*
[] -> optional
() -> Any of the symbols is correct

PROGRAM := EXPRESSION*

EXPRESSION := BLOCK | FUN_DEF | RETURN | BINARY

DEF := FUN_DEF | VAR_DEF
BLOCK := "{" STATEMENT "}"
FUN_DEF := IDENTIFIER "(" ARGS ")"  "=" EXPRESSION
ARGS := [ IDENTIFIER ]*
RETURN := RETURN [EXPRESSION]

BINARY := AND ["and" AND]*
AND := OR ["or" OR]*
OR := EQUALITY ["||" EQUALITY]
EQUALITY:=  COMPARISON [ ("!= " | "==" ) COMPARISON]* 
COMPARISON := MODULO [("<" | "<=" | ">" | ">=") MODULO]*
MODULO := SUM ["%" SUM]*
SUM := MULTIPLICATION [  "-" | "+"  ) MULTIPLICATION] *
MULTIPLICATION := UNARY [("*" | "/") UNARY]*
UNARY := ("+" |"-" | "!") UNARY | EXPONENTIATION
EXPONENTIATION = ASSIGN ["^" ASSIGN]*
ASSIGN := CALL ["=" EXPRESSION]
CALL := LITERAL ["( [ CALL_ARGS ] )"]
LITERAL := NUMBER | IDENTIFIER  | "(" EXPRESSION ")"
CALL_ARGS := EXPRESSION | EXPRESSION "," CALL_ARGS
*/

namespace CL {

class Parser {
private:

    int lexical_scope = 0;

	std::vector<Token> m_parsed_tokens;
	size_t m_current_token;
	Lexer m_lexer;

	template<typename T, typename... Tokens>
	bool match(T t, Tokens... ts);
	bool match(TokenType type);

	template<typename T, typename... Tokens>
	Token consume(const std::string &error, T t, Tokens... ts);
	Token consume(const std::string &error, TokenType type);

	StatementPtr statement();
	ExprPtr expression();
	StatementPtr block_statement();
	ExprPtr return_expression();
	ExprPtr module_expression();
	StatementPtr if_statement();
	StatementPtr while_statement();
	StatementPtr for_statement();
	ExprPtr and_expr();
	ExprPtr or_expr();
	ExprPtr bitwise();
	ExprPtr equality_expression();
	ExprPtr comparison();
	ExprPtr shift();
	ExprPtr sum();
	ExprPtr multiplication();
	ExprPtr unary();
	ExprPtr exponentiation();
	ExprPtr assign();
	ExprPtr get();
	ExprPtr call();
	ExprPtr literal();
    StatementPtr fun_statement();

	ExprPtr dict_expression();
	ExprPtr list_expression();

	bool is_in_global_scope() const { return lexical_scope == 0; }
	bool match_expression_begin();
	Names arg_names();
	ExprList get_arguments();

	Token next();
	Token &peek();
	Token previous();
	static void throw_exception(const std::string &why, const Token &cause);

public:
	explicit Parser(Lexer lexer);

	StatementList parse_all();
};
} // namespace CL
