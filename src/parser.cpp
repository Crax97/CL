#include "parser.hpp"
#include "commons.hpp"
#include "exceptions.hpp"
#include "nodes.hpp"
#include "tokens.hpp"
#include <memory>
#include <sstream>
#include <utility>

namespace CL {

BinaryOp token_type_to_binary_opcode(TokenType type) {
	switch (type) {
		case TokenType::Plus:return BinaryOp::Addition;
		case TokenType::Minus:return BinaryOp::Subtraction;
		case TokenType::Star:return BinaryOp::Multiplication;
		case TokenType::Slash:return BinaryOp::Division;
		case TokenType::Percent:return BinaryOp::Modulo;
		case TokenType::Equals:return BinaryOp::Equals;
		case TokenType::Not_Equals:return BinaryOp::Not_Equals;
		case TokenType::Less:return BinaryOp::Less;
		case TokenType::Less_Or_Equals:return BinaryOp::Less_Equals;
		case TokenType::Greater:return BinaryOp::Greater;
		case TokenType::Greater_Or_Equals:return BinaryOp::Greater_Equals;
		case TokenType::And:return BinaryOp::And;
		case TokenType::Or:return BinaryOp::Or;
		default: NOT_REACHED();
	}
	NOT_REACHED();
}

UnaryOp token_type_to_unary_opcode(TokenType type) {
	switch (type) {
		case TokenType::Plus:return UnaryOp::Identity;
		case TokenType::Minus:
		case TokenType::Not:return UnaryOp::Negation;
		default: NOT_REACHED();
	}
	NOT_REACHED();
}

class ParsingException : public CLException {
private:
	Token m_error_token;

public:
	ParsingException(Token error_token, std::string_view error_message)
		: m_error_token(std::move(std::move(error_token))),
		  CLException(std::string(error_message)) {
	}
};

Parser::Parser(Lexer lexer)
	: m_lexer(std::move(lexer)), m_current_token(0) {
}

template<typename T, typename... Tokens>
bool Parser::match(T t, Tokens... ts) {
	return match(t) || match(ts...);
}

bool Parser::match(TokenType type) {
	auto &p = peek();
	if(p.get_type() == type) {
		next();
		return true;
	}
	return false;
}

template<typename T, typename... Tokens>
Token Parser::consume(const std::string &error, T t, Tokens... ts) {
	if(peek().get_type() == t) {
		return next();
	} else {
		return consume(ts...);
	}
}

Token Parser::consume(const std::string &error, TokenType type) {
	if(peek().get_type() == type) {
		return next();
	} else {
		throw_exception(error, peek());
		return peek();
	}
}

Token Parser::next() {
	auto token = peek();
	m_current_token++;
	return token;
}

Token &Parser::peek() {
	if(m_current_token == m_parsed_tokens.size()) {
		auto token = m_lexer.next();
		m_parsed_tokens.push_back(token);
	}
	return m_parsed_tokens[m_current_token];
}

Token Parser::previous() {
	if(m_current_token == 0) {
		throw_exception("No tokens have been parsed yet in the stream",
						Token(TokenType::Eof, 0, 0, "<Empty stream so far>"));
	}

	return m_parsed_tokens[m_current_token - 1];
}

void Parser::throw_exception(const std::string &why, const Token &cause) {
	std::stringstream stream;
	stream << "Syntax error at " << cause.get_line() << ":"
		   << cause.get_column() - 1 << ":\n";
	stream << "\t" << why << "\n";
	std::string dashes = std::string(
		cause.get_source_line().size() > cause.get_column() ?
		cause.get_source_line().size() : cause.get_column(), '-');
	dashes[cause.get_column() - 1] = '^';
	stream << "│ " << cause.get_source_line() << "\n";
	stream << "└>" << dashes << "\n";
	throw ParsingException(cause, stream.str());
}

StatementList Parser::parse_all() {
	auto list = StatementList();
	while (!m_lexer.is_at_end()
		&& m_lexer.peek().get_type() != TokenType::Eof) {
		auto next_statement = statement();
		list.push_back(std::move(next_statement));
	}
	return list;
}

StatementPtr Parser::statement() {
    if (match(TokenType::Left_Curly_Brace)) {
        return block_statement();
    } else if (match(TokenType::While)) {
        return while_statement();
    } else if (match(TokenType::For)) {
        return for_statement();
    } else if (match(TokenType::If)) {
        return if_statement();
    } else if(match(TokenType::Fun)) {
        return fun_statement();
    }
    return std::make_unique<ExpressionStatement>(expression());
}

ExprPtr Parser::expression() {
    if(match(TokenType::Return)) {
        return return_expression();
    } else if(match(TokenType::Module)) {
        return module_expression();
    } else if(match(TokenType::Continue)) {
		return std::make_shared<ContinueExpression>();
	} else if(match(TokenType::Break)) {
		return std::make_shared<BreakExpression>();
	}
	return and_expr();
}

StatementPtr Parser::if_statement() {
	auto cond = expression();
	auto body = statement();
	StatementPtr else_block = nullptr;
	if(match(TokenType::Else)) {
		else_block = statement();
	}
	return std::make_unique<IfStatement>(std::move(cond),
										  std::move(body),
										  std::move(else_block));
}

ExprPtr Parser::module_expression() {
	consume("Module definitions must start with a {",
			TokenType::Left_Curly_Brace);
	ExprList list;
	while (!match(TokenType::Right_Curly_Brace)) {
		list.push_back(expression());
	}
	return std::make_unique<ModuleExpression>(list);
}

StatementPtr Parser::while_statement() {
	auto cond = expression();
	auto body = statement();
	return std::make_unique<WhileStatement>(cond, body);
}

StatementPtr Parser::for_statement() {
	auto name = consume("For expressions start with an identifier",
						TokenType::Identifier).get<String>();
	consume("For expressions must have an \"in\" after the identifier",
			TokenType::In);
	auto iterator = expression();
	auto body = statement();
	return std::make_unique<ForStatement>(name, iterator, body);
}

StatementPtr Parser::block_statement() {
	auto list = StatementList();
	while (!match(TokenType::Right_Curly_Brace)) {
		list.push_back(statement());
	}
	return std::make_unique<BlockStatement>(std::move(list));
}

Names Parser::arg_names() {
	consume("A list of arguments begins with a (", TokenType::Left_Brace);
	auto args = Names();
	if(!match(TokenType::Right_Brace)) {
		do {
			auto tok = consume("Arguments can only be identifiers",
							   TokenType::Identifier);
			args.push_back(tok.get<std::string>());
			match(TokenType::Comma);
		} while (!match(TokenType::Right_Brace));
	}
	return args;
}

ExprPtr Parser::return_expression() {
	if(match_expression_begin()) {
		return std::make_unique<ReturnExpression>(expression());
	}
	return std::make_unique<ReturnExpression>(nullptr);
}

ExprPtr Parser::and_expr() {
	auto left_expr = or_expr();
	while (match(TokenType::And)) {
		auto right = or_expr();
		left_expr = std::make_unique<AndExpression>(std::move(left_expr),
													std::move(right));
	}
	return left_expr;
}

ExprPtr Parser::or_expr() {
	auto left_expr = equality_expression();
	while (match(TokenType::Or)) {
		auto right = equality_expression();
		left_expr = std::make_unique<OrExpression>(std::move(left_expr),
												   std::move(right));
	}
	return left_expr;
}

ExprPtr Parser::equality_expression() {
	auto left_expr = comparison();
	while (match(TokenType::Not_Equals, TokenType::Equals)) {
		auto type = previous().get_type();
		auto right = comparison();
		left_expr = std::make_unique<BinaryExpression>(std::move(left_expr),
													   token_type_to_binary_opcode(
														   type),
													   std::move(right));
	}
	return left_expr;
}

ExprPtr Parser::comparison() {
	auto left_expr = shift();
	while (match(TokenType::Less_Or_Equals,
				 TokenType::Greater_Or_Equals,
				 TokenType::Less,
				 TokenType::Greater)) {
		auto type = previous().get_type();
		auto right = shift();
		left_expr = std::make_unique<BinaryExpression>(std::move(left_expr),
													   (token_type_to_binary_opcode(
														   type)),
													   std::move(right));
	}
	return left_expr;
}

ExprPtr Parser::shift() {
	auto left_expr = sum();
	while (match(TokenType::Right_Shift, TokenType::Left_Shift)) {
		auto type = previous().get_type();
		auto right = sum();
		left_expr = std::make_unique<BinaryExpression>(std::move(left_expr),
													   (token_type_to_binary_opcode(
														   type)),
													   std::move(right));
	}
	return left_expr;
}

ExprPtr Parser::sum() {
	auto left_expr = multiplication();
	while (match(TokenType::Plus, TokenType::Minus)) {
		auto type = previous().get_type();
		auto right = multiplication();
		left_expr = std::make_unique<BinaryExpression>(std::move(left_expr),
													   token_type_to_binary_opcode(
														   type),
													   std::move(right));
	}
	return left_expr;
}

ExprPtr Parser::multiplication() {
	auto left_expr = unary();
	while (match(TokenType::Star, TokenType::Slash, TokenType::Percent)) {
		auto type = previous().get_type();
		auto right = unary();
		left_expr = std::make_unique<BinaryExpression>(std::move(left_expr),
													   (token_type_to_binary_opcode(
														   type)),
													   std::move(right));
	}
	return left_expr;
}

ExprPtr Parser::unary() {
	while (match(TokenType::Plus, TokenType::Minus, TokenType::Not)) {
		auto type = previous().get_type();
		auto expr = unary();
		return std::make_unique<UnaryExpression>(std::move(expr),
												 token_type_to_unary_opcode(type));
	}
	auto left_expr = exponentiation();
	return left_expr;
}

ExprPtr Parser::exponentiation() {
	auto left = assign();
	while (match(TokenType::Xor)) {
		auto exponent = unary();
		left = std::make_unique<BinaryExpression>(std::move(left),
												  BinaryOp::Exponentiation,
												  std::move(exponent));
	}
	return left;
}

ExprPtr Parser::assign() {
	ExprPtr expr = call();
	if(match(TokenType::Dot, TokenType::Left_Square_Brace)) {
		do {
			ExprPtr what = nullptr;
			if(previous().get_type() == TokenType::Left_Square_Brace) {
				what = expression();
				consume("Indexing expressions end with a ]",
						TokenType::Right_Square_Brace);
			} else {
				auto next =
					consume("Named indexing expressions expect an identifier",
							TokenType::Identifier).get<std::string>();
				what = std::make_unique<StringExpression>(next);
			}
			if(match(TokenType::Assign)) {
				expr =
					std::make_unique<SetExpression>(expr, what, expression());
			} else {
				expr = std::make_unique<GetExpression>(expr, what);
			}
		} while (match(TokenType::Dot, TokenType::Left_Square_Brace));
		return expr;
	}

	while (peek().get_type() == TokenType::Assign) {
		auto p = previous();
		if(p.get_type() != TokenType::Identifier) {
			throw_exception("Invalid assign target!", p);
		}
		auto id = previous().get<String>();
		next();
		expr = std::make_unique<AssignExpression>(id, std::move(expression()));
	}
	return expr;
}


ExprPtr Parser::call() {
    auto left = literal();
    while (match(TokenType::Left_Brace)) {
        ExprList args = get_arguments();

        left = std::make_unique<FunCallExpression>(
                std::move(left),
                std::move(args));
    }
    return left;
}

ExprList Parser::get_arguments() {
    ExprList list;
    while (!match(TokenType::Right_Brace)) {
        list.push_back(expression());
        match(TokenType::Comma);
    }
    return list;
}

ExprPtr Parser::literal() {
	auto next_token = peek();
	if(match(TokenType::Number)) {
		return std::make_unique<NumberExpression>(next_token.get<Number>());
	} else if(match(TokenType::String)) {
		return std::make_unique<StringExpression>(next_token.get<String>());
	} else if(match(TokenType::Identifier)) {
		auto name = next_token.get<std::string>();
		return std::make_unique<VarExpression>(name);
	} else if(match(TokenType::Left_Brace)) {
		auto expr = expression();
		consume("Grouping expressions must end with a )",
				TokenType::Right_Brace);
		return expr;
	} else if(match(TokenType::Dict)) {
		return dict_expression();
	} else if(match(TokenType::List)) {
		return list_expression();
	}

	auto prev = previous();
	auto next_string = next_token.to_string();
	auto previous_string = prev.to_string();
	throw_exception("Cannot parse " + next_string + " as an expression!\n",
					next_token);
	return nullptr;
}

StatementPtr Parser::fun_statement() {
	auto names = arg_names();
	consume("function definitions expect a -> after the fun keyword",
			TokenType::Arrow);
	auto body = statement();
	return std::make_unique<FunDefStatement>(names, std::move(body));
}

ExprPtr Parser::dict_expression() {
	consume("dict expressions begin with a { after the dict keyword",
			TokenType::Left_Curly_Brace);
	auto expressions = std::vector<std::pair<ExprPtr, ExprPtr>>();

	while (!match(TokenType::Right_Curly_Brace)) {
		auto l = expression();
		consume("Key and values are separated by a :", TokenType::Double_Dots);
		auto r = expression();
		expressions.emplace_back(l, r);
	}

	return std::make_unique<DictExpression>(expressions);
}

ExprPtr Parser::list_expression() {
	consume("list expressions begin with a [ after the list keyword",
			TokenType::Left_Square_Brace);
	auto expressions = ExprList();
	while (!match(TokenType::Right_Square_Brace)) {
		expressions.push_back(expression());
		match(TokenType::Comma);
	}

	return std::make_unique<ListExpression>(expressions);
}

bool Parser::match_expression_begin() {
	auto p = peek().get_type();
	return (p == TokenType::Identifier || p == TokenType::Number
		|| p == TokenType::Plus || p == TokenType::Minus ||
		p == TokenType::Not || p == TokenType::Left_Brace
		|| p == TokenType::Self);
}

} // namespace CL
