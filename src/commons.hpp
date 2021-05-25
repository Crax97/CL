#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#define TODO()                                                                 \
    {                                                                          \
    std::cerr << "TODO Reached on file " << std::string(__FILE__) << "\n"; \
    std::cerr << "\trunning function " << std::string(__func__);           \
    std::cerr << " at line " << std::to_string(__LINE__) << "\n";          \
    throw std::exception();                                                \
    }

#define NOT_REACHED() TODO()

namespace CL {
class RuntimeValue;

class Callable;
class Expression;
class Statement;
class Indexable;
class StackedEnvironment;

using RuntimeEnvPtr = std::shared_ptr<StackedEnvironment>;
using IndexablePtr = std::shared_ptr<Indexable>;

using Number = double;
using String = std::string;
using Args = std::vector<RuntimeValue>;
using Names = std::vector<std::string>;
using ExprPtr = std::shared_ptr<Expression>;
using ExprList = std::vector<ExprPtr>;
using StatementPtr = std::shared_ptr<Statement>;
using StatementList = std::vector<StatementPtr>;
using CallablePtr = std::shared_ptr<Callable>;
using FunctionCallback = std::function<std::optional<RuntimeValue>(const Args &args)>;
using VoidFunctionCallback = std::function<void(const Args &args)>;

enum class BinaryOp {
	Multiplication,
	Division,
	Modulo,
	Addition,
	Subtraction,
	Exponentiation,
	Equals,
	Not_Equals,
	Less,
	Less_Equals,
	Greater,
	Greater_Equals,
	And,
	Or,
};

enum class UnaryOp {
	Negation,
	Identity,
};

enum class TokenType {
	Eof,
	Newline,
	Number,
	String,
	Module,
	Plus,
	Minus,
	Star,
	Slash,
	Percent,
	Equals,
	Dot,
	Comma,
	Double_Dots,
	Point_Comma,
	Not_Equals,
	Left_Brace,
	Left_Curly_Brace,
	Left_Square_Brace,
	Right_Brace,
	Right_Curly_Brace,
	Right_Square_Brace,
	Less,
	Less_Or_Equals,
	Greater,
	Greater_Or_Equals,
	Left_Shift,
	Right_Shift,
	Arrow,
	Ampersand,
	Pipe,
	And,
	Or,
	Not,
	Xor,
	Assign,
	Let,
	Fun,
	Global,
	Self,
	Identifier,
	If,
	Else,
	While,
	For,
	In,
	Return,
	Continue,
	Break,
	Dict,
	List,
    Expose,
};

std::string token_type_to_string(TokenType type) noexcept;
std::string binary_op_to_string(BinaryOp op) noexcept;
std::string unary_op_to_string(UnaryOp op) noexcept;
} // namespace CL
