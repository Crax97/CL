#include "parser.hpp"
#include "commons.hpp"
#include "exceptions.hpp"
#include "nodes.hpp"
#include "tokens.hpp"
#include "value.hpp"
#include <memory>
#include <sstream>

namespace Calculator {

BinaryOp token_type_to_binary_opcode(TokenType type)
{
    switch (type) {
    case TokenType::Plus:
	return BinaryOp::Addition;
	break;
    case TokenType::Minus:
	return BinaryOp::Subtraction;
	break;
    case TokenType::Star:
	return BinaryOp::Multiplication;
	break;
    case TokenType::Slash:
	return BinaryOp::Division;
	break;
    case TokenType::Percent:
	return BinaryOp::Division;
	break;
    case TokenType::Equals:
	return BinaryOp::Equals;
    case TokenType::Not_Equals:
	return BinaryOp::Not_Equals;
	break;
    case TokenType::Less:
	return BinaryOp::Less;
	break;
    case TokenType::Less_Or_Equals:
	return BinaryOp::Less_Equals;
	break;
    case TokenType::Greater:
	return BinaryOp::Greater;
	break;
    case TokenType::Greater_Or_Equals:
	return BinaryOp::Greater_Equals;
	break;
    case TokenType::And:
	return BinaryOp::And;
    case TokenType::Or:
	return BinaryOp::Or;
	break;
    default:
	NOT_REACHED();
    }
    NOT_REACHED();
    return BinaryOp::Addition;
}

UnaryOp token_type_to_unary_opcode(TokenType type)
{
    switch (type) {
    case TokenType::Plus:
	return UnaryOp::Identity;
    case TokenType::Minus:
    case TokenType::Not:
	return UnaryOp::Negation;
    default:
	NOT_REACHED();
    }
    NOT_REACHED();
    return UnaryOp::Identity;
}

class ParsingException : public CLException {
private:
    Token m_error_token;

public:
    ParsingException(Token error_token, std::string_view error_message)
	: m_error_token(error_token)
	, CLException(std::string(error_message))
    {
    }
};

Parser::Parser(Lexer lexer)
    : m_lexer(lexer)
    , m_current_token(0)
{
}

template <typename T, typename... Tokens>
bool Parser::match(T t, Tokens... ts)
{
    return match(t) || match(ts...);
}
bool Parser::match(TokenType type)
{
    auto& p = peek();
    return p.get_type() == type;
}

template <typename T, typename... Tokens>
Token Parser::consume(T t, Tokens... ts)
{
    if (peek().get_type() == t) {
	return consume(t);
    } else {
	return consume(ts...);
    }
}
Token Parser::consume(TokenType type)
{
    auto token = peek();
    if (match(type)) {
	next();
	return token;
    } else {
	auto msg = "Expected " + token_type_to_string(type) + ", found " + peek().to_string();
	throw_exception(msg, peek());
	return peek();
    }
}

Token Parser::next()
{
    auto token = peek();
    m_current_token++;
    return token;
}

Token& Parser::peek()
{
    if (m_current_token == m_parsed_tokens.size()) {
	auto token = m_lexer.next();
	m_parsed_tokens.push_back(token);
    }
    return m_parsed_tokens[m_current_token];
}

Token Parser::previous()
{
    if (m_current_token == 0) {
	throw_exception("No tokens have been parsed yet in the stream", Token(TokenType::Eof, 0, 0, "<Empty stream so far>"));
    }

    return m_parsed_tokens[--m_current_token];
}

void Parser::throw_exception(const std::string& why, const Token& cause)
{
    std::stringstream stream;
    stream << "Syntax error "
	   << "at " << cause.get_line() << ":" << cause.get_column() << ": " << why << "\n";
    std::string dashes = std::string(cause.get_source_line().size(), '-');
    dashes[cause.get_column() - 2] = '^';
    stream << "| " << cause.get_source_line() << "\n";
    stream << "|-" << dashes << "\n";
    throw ParsingException(cause, stream.str());
}

ExprList Parser::parse_all()
{
    auto list = ExprList();
    while (!m_lexer.is_at_end() && m_lexer.peek().get_type() != TokenType::Eof) {
	auto next_statement = expression();
	list.push_back(std::move(next_statement));
    }
    return list;
}

ExprPtr Parser::expression()
{
    if (match(TokenType::Left_Curly_Brace)) {
	return block_expression();
    } else if (match(TokenType::Return)) {
	return return_expression();
    } else if (match(TokenType::Module)) {
	return module_expression();
    } else if (match(TokenType::While)) {
	return while_expression();
    } else if (match(TokenType::For)) {
	return for_expression();
    }
    auto expr = and_expr();
    if (match(TokenType::If)) {
	consume(TokenType::If);
	auto cond = expression();
	if (match(TokenType::Else)) {
	    consume(TokenType::Else);
	    return std::make_unique<IfExpression>(std::move(cond), std::move(expr), expression());
	} else {
	    return std::make_unique<IfExpression>(std::move(cond), std::move(expr), nullptr);
	}
    }
    return expr;
}

ExprPtr Parser::module_expression()
{
    consume(TokenType::Module);
    consume(TokenType::Left_Curly_Brace);
    ExprList list;
    while (!match(TokenType::Right_Curly_Brace)) {
	list.push_back(expression());
    }
    consume(TokenType::Right_Curly_Brace);
    return std::make_unique<ModuleExpression>(list);
}

ExprPtr Parser::while_expression()
{
    consume(TokenType::While);
    auto cond = expression();
    auto body = expression();
    return std::make_unique<WhileExpression>(cond, body);
}

ExprPtr Parser::for_expression()
{
    consume(TokenType::For);
    auto name = consume(TokenType::Identifier).get<String>();
    consume(TokenType::In);
    auto iterator = expression();
    auto body = expression();

    return std::make_unique<ForExpression>(name, iterator, body);
}

ExprPtr Parser::block_expression()
{
    consume(TokenType::Left_Curly_Brace);
    auto list = ExprList();
    while (!match(TokenType::Right_Curly_Brace)) {
	list.push_back(expression());
    }
    consume(TokenType::Right_Curly_Brace);
    return std::make_unique<BlockExpression>(std::move(list));
}
Names Parser::arg_names()
{
    consume(TokenType::Left_Brace);
    auto args = Names();
    if (!match(TokenType::Right_Brace)) {
	do {
	    auto tok = consume(TokenType::Identifier);
	    args.push_back(tok.get<std::string>());
	    if (match(TokenType::Comma)) {
		consume(TokenType::Comma);
	    }
	} while (!match(TokenType::Right_Brace));
    }
    consume(TokenType::Right_Brace);
    return args;
}

ExprPtr Parser::return_expression()
{
    consume(TokenType::Return);
    if (match_expression_begin()) {
	return std::make_unique<ReturnExpression>(expression());
    }
    return std::make_unique<ReturnExpression>(nullptr);
}
ExprPtr Parser::and_expr()
{
    auto left_expr = or_expr();
    while (match(TokenType::And)) {
	consume(TokenType::And);
	auto right = or_expr();
	left_expr = std::make_unique<AndExpression>(std::move(left_expr), std::move(right));
    }
    return left_expr;
}
ExprPtr Parser::or_expr()
{
    auto left_expr = equality_expression();
    while (match(TokenType::Or)) {
	consume(TokenType::Or);
	auto right = equality_expression();
	left_expr = std::make_unique<OrExpression>(std::move(left_expr), std::move(right));
    }
    return left_expr;
}

ExprPtr Parser::equality_expression()
{
    auto left_expr = comparison();
    while (match(TokenType::Not_Equals, TokenType::Equals)) {
	auto type = consume(TokenType::Not_Equals, TokenType::Equals).get_type();
	auto right = comparison();
	left_expr = std::make_unique<BinaryExpression>(std::move(left_expr), token_type_to_binary_opcode(type), std::move(right));
    }
    return left_expr;
}
ExprPtr Parser::comparison()
{
    auto left_expr = shift();
    while (match(TokenType::Less_Or_Equals, TokenType::Greater_Or_Equals, TokenType::Less, TokenType::Greater)) {
	auto type = consume(TokenType::Less_Or_Equals, TokenType::Greater_Or_Equals, TokenType::Less, TokenType::Greater).get_type();
	auto right = shift();
	left_expr = std::make_unique<BinaryExpression>(std::move(left_expr), (token_type_to_binary_opcode(type)), std::move(right));
    }
    return left_expr;
}
ExprPtr Parser::shift()
{
    auto left_expr = modulo();
    while (match(TokenType::Right_Shift, TokenType::Left_Shift)) {
	auto type = consume(TokenType::Right_Shift, TokenType::Left_Shift)
			.get_type();
	auto right = modulo();
	left_expr = std::make_unique<BinaryExpression>(std::move(left_expr), (token_type_to_binary_opcode(type)), std::move(right));
    }
    return left_expr;
}
ExprPtr Parser::modulo()
{
    auto left_expr = sum();
    while (match(TokenType::Percent)) {
	auto type = consume(TokenType::Percent)
			.get_type();
	auto right = sum();
	left_expr = std::make_unique<BinaryExpression>(std::move(left_expr), (token_type_to_binary_opcode(type)), std::move(right));
    }
    return left_expr;
}
ExprPtr Parser::sum()
{
    auto left_expr = multiplication();
    while (match(TokenType::Plus, TokenType::Minus)) {
	auto type = consume(TokenType::Plus, TokenType::Minus)
			.get_type();
	auto right = multiplication();
	left_expr = std::make_unique<BinaryExpression>(std::move(left_expr), token_type_to_binary_opcode(type), std::move(right));
    }
    return left_expr;
}

ExprPtr Parser::multiplication()
{
    auto left_expr = exponentiation();
    while (match(TokenType::Star, TokenType::Slash)) {
	auto type = consume(TokenType::Star, TokenType::Slash)
			.get_type();
	auto right = exponentiation();
	left_expr = std::make_unique<BinaryExpression>(std::move(left_expr), (token_type_to_binary_opcode(type)), std::move(right));
    }
    return left_expr;
}

ExprPtr Parser::exponentiation()
{
    auto left = unary();
    while (match(TokenType::Xor)) {
	consume(TokenType::Xor);
	auto exponent = unary();
	left = std::make_unique<BinaryExpression>(std::move(left), BinaryOp::Exponentiation, std::move(exponent));
    }
    return left;
}

ExprPtr Parser::unary()
{
    while (match(TokenType::Plus, TokenType::Minus, TokenType::Not)) {
	auto token = consume(TokenType::Plus, TokenType::Minus, TokenType::Not);
	auto expr = unary();
	auto type = token.get_type();
	return std::make_unique<UnaryExpression>(std::move(expr), token_type_to_unary_opcode(type));
    }
    auto left_expr = call();
    return left_expr;
}
ExprList Parser::get_arguments()
{
    ExprList list;
    consume(TokenType::Left_Brace);
    while (!match(TokenType::Right_Brace)) {
	list.push_back(expression());
	if (match(TokenType::Comma)) {
	    consume(TokenType::Comma);
	}
    }
    consume(TokenType::Right_Brace);
    return list;
}

ExprPtr Parser::call()
{
    auto left = assign();
    while (match(TokenType::Left_Brace)) {
	ExprList args = get_arguments();

	left = std::make_unique<FunCallExpression>(
	    std::move(left),
	    std::move(args));
    }
    return left;
}

ExprPtr Parser::assign()
{
    auto expr = literal();
    if (match(TokenType::Dot)) {
	while (match(TokenType::Dot)) {
	    consume(TokenType::Dot);
	    auto next = consume(TokenType::Identifier).get<std::string>();
	    if (match(TokenType::Assign)) {
		consume(TokenType::Assign);
		expr = std::make_unique<SetExpression>(expr, std::make_unique<StringExpression>(next), expression());
	    } else {
		expr = std::make_unique<GetExpression>(expr, std::make_unique<StringExpression>(next));
	    }
	}
	return expr;
    }
    while (match(TokenType::Assign)) {
	auto prev = previous();
	next();
	consume(TokenType::Assign);
	if (prev.get_type() == TokenType::Identifier) {
	    auto name = prev.get<std::string>();
	    auto expr = expression();
	    return std::make_unique<AssignExpression>(name, std::move(expr));
	} else if (prev.get_type() == TokenType::Right_Brace) {
	    throw_exception("Invalid assign target!", prev);
	}
    }
    return expr;
}

ExprPtr Parser::literal()
{
    auto next_token = peek();
    if (match(TokenType::Number)) {
	next();
	return std::make_unique<NumberExpression>(next_token.get<Number>());
    } else if (match(TokenType::String)) {
	consume(TokenType::String);
	return std::make_unique<StringExpression>(next_token.get<String>());
    } else if (match(TokenType::Identifier)) {
	auto name = next_token.get<std::string>();
	next();
	return std::make_unique<VarExpression>(name);
    } else if (match(TokenType::Fun)) {
	consume(TokenType::Fun);
	auto names = arg_names();
	consume(TokenType::Arrow);
	auto body = expression();
	return std::make_unique<FunDef>(names, std::move(body));
    } else if (match(TokenType::Left_Brace)) {
	consume(TokenType::Left_Brace);
	auto expr = expression();
	consume(TokenType::Right_Brace);
	return expr;
    }

    auto prev = previous();
    auto next_string = next_token.to_string();
    auto previous_string = prev.to_string();
    throw_exception("Cannot parse " + next_string + " as an expression!\n", next_token);
    return nullptr;
}

bool Parser::match_expression_begin()
{
    auto p = peek().get_type();
    return (p == TokenType::Identifier || p == TokenType::Number || p == TokenType::Plus || p == TokenType::Minus || p == TokenType::Not || p == TokenType::Left_Brace || p == TokenType::Self);
}
} // namespace CL
