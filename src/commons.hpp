#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <variant>

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
class Indexable;
class StackedEnvironment;
class CompilationStackFrame;
class FunctionFrame;

using RuntimeEnvPtr = std::shared_ptr<StackedEnvironment>;
using IndexablePtr = std::shared_ptr<Indexable>;

using Number = double;
using String = std::string;
using Args = std::vector<RuntimeValue>;
using Names = std::vector<std::string>;
using ExprPtr = std::shared_ptr<Expression>;
using ExprList = std::vector<ExprPtr>;
using CallablePtr = std::shared_ptr<Callable>;
using FunctionCallback = std::function<std::optional<RuntimeValue>(const Args &args)>;
using VoidFunctionCallback = std::function<void(const Args &args)>;
using LiteralValue = std::variant<Number, String, std::shared_ptr<FunctionFrame>>;

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
enum class Opcode : uint8_t {
    Nop                 = 0x00, // Does nothing, wasting one vm cycle
    Load_Literal        = 0x01, // push(literals[read32])
    Load                = 0x02, // push(memory[names[read16()]])
    Store               = 0x03, // memory[names[read16()]] = stack[0], pops stack
    Set                 = 0x04, // stack[-2][stack[-1]] = stack[0], pops stack, pushes stack[0]
    Get                 = 0x05, // push stack[-1][stack[0]], pops stack,
    List                = 0x06, // n, 32 bit integer with the number of elements belonging to the list
    Dict                = 0x07, // n, 32 bit integer with the number of associations belonging to the list
    Neg                 = 0x08, // push(-stack[0]), pops stack
    Add                 = 0x09, // push(stack[0] + stack[-1]), pops stack
    Sub                 = 0x0A, // push(stack[0] - stack[-1]), pops stack
    Mul                 = 0x0B, // push(stack[0] * stack[-1]), pops stack
    Div                 = 0x0C, // push(stack[0] / stack[-1]), pops stack
    Mod                 = 0x0D, // push(stack[0] % stack[-1]), pops stack
    Pow                 = 0x0E, // push(stack[0] ^ stack[-1]), pops stack
    Less                = 0x10, // push(stack[0] < stack[-1]), pops stack
    Less_Eq             = 0x11, // push(stack[0] <= stack[-1]), pops stack
    Greater             = 0x12, // push(stack[0] > stack[-1]), pops stack
    Greater_Eq          = 0x13, // push(stack[0] >= stack[-1]), pops stack
    Eq                  = 0x14, // push(stack[0] == stack[-1]), pops stack
    Neq                 = 0x15, // push(stack[0] != stack[-1]), pops stack
    True                = 0x16, // push(truthy(stack[0])), pops stack
    Push_Frame          = 0x20, // push new stack frame
    Pop_Frame           = 0x21, // pops last stack frame
    Jump_True           = 0x22, // Jumps to read32() if true
    Jump_False          = 0x23, // Jumps to read32() if false
    Jump                = 0x24, // Jumps to read32()
    Call                = 0x25, // k -> byte with number of arguments, stack[0](arg_1...arg_k) where arg_i is the result of the i-eth expression after the current opcode
    Module              = 0x26, // k -> the next k expressions belong to the module
    Return              = 0x27, // stack_frames[-1].return_value = stack[0]; stack_frames.pop();
    Break               = 0x28, // stack_frames.pop();
    Continue            = 0x29, // stack_frames.pop();
    Get_Iter            = 0x2A, // push(get_iterator(stack[0])), pops stack[0]
    Iter_Has_Next       = 0x2B, // push(has_next(stack[0])), pops nothing
    Get_Iter_Next       = 0x2C, // push(get_next(stack[0])), pops nothing
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

};

std::string token_type_to_string(TokenType type) noexcept;
std::string binary_op_to_string(BinaryOp op) noexcept;
std::string unary_op_to_string(UnaryOp op) noexcept;
std::string opcode_to_string(Opcode op);
Opcode opcode_from_binary(BinaryOp op);
Opcode opcode_from_unary(UnaryOp op);
Opcode byte_to_opcode(uint8_t code);

} // namespace CL
