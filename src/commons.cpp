#include "commons.hpp"

namespace Calculator {
std::string binary_op_to_string(BinaryOp op) noexcept
{
    switch (op) {
    case BinaryOp::Addition:
	return "+";
    case BinaryOp::Subtraction:
	return "-";
    case BinaryOp::Multiplication:
	return "*";
    case BinaryOp::Division:
	return "/";
    case BinaryOp::Modulo:
	return "%";
    case BinaryOp::Exponentiation:
	return "^";
    case BinaryOp::And:
	return " and ";
    case BinaryOp::Or:
	return " or ";
    case BinaryOp::Less:
	return "<";
    case BinaryOp::Less_Equals:
	return "<=";
    case BinaryOp::Greater:
	return ">";
    case BinaryOp::Greater_Equals:
	return ">=";
    case BinaryOp::Equals:
	return "==";
    case BinaryOp::Not_Equals:
	return "!=";
    }
    return "Unrecognized";
}
std::string unary_op_to_string(UnaryOp op) noexcept
{
    switch (op) {
    case UnaryOp::Identity:
	return "+";
    case UnaryOp::Negation:
	return "-";
    }
    return "Unrecognized";
}

}
