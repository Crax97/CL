#include "commons.hpp"
#include "exceptions.hpp"

namespace CL {
std::string binary_op_to_string(BinaryOp op) noexcept {
	switch (op) {
		case BinaryOp::Addition: return "+";
		case BinaryOp::Subtraction: return "-";
		case BinaryOp::Multiplication: return "*";
		case BinaryOp::Division: return "/";
		case BinaryOp::Modulo: return "%";
		case BinaryOp::Exponentiation: return "^";
		case BinaryOp::And: return " and ";
		case BinaryOp::Or: return " or ";
		case BinaryOp::Less: return "<";
		case BinaryOp::Less_Equals: return "<=";
		case BinaryOp::Greater: return ">";
		case BinaryOp::Greater_Equals: return ">=";
		case BinaryOp::Equals: return "==";
		case BinaryOp::Not_Equals: return "!=";
	}
	return "Unrecognized";
}
std::string unary_op_to_string(UnaryOp op) noexcept {
	switch (op) {
		case UnaryOp::Identity: return "+";
		case UnaryOp::Negation: return "-";
	}
	return "Unrecognized";
}

Opcode opcode_from_binary(BinaryOp op) {
    switch(op) {
        case BinaryOp::Addition: return Opcode::Add;
        case BinaryOp::Subtraction: return Opcode::Sub;
        case BinaryOp::Multiplication: return Opcode::Mul;
        case BinaryOp::Division: return Opcode::Div;
        case BinaryOp::Modulo: return Opcode::Mod;
        case BinaryOp::Greater: return Opcode::Greater;
        case BinaryOp::Greater_Equals: return Opcode::Greater_Eq;
        case BinaryOp::Less: return Opcode::Less;
        case BinaryOp::Less_Equals: return Opcode::Less_Eq;
        case BinaryOp::Equals: return Opcode::Eq;
        case BinaryOp::Not_Equals: return Opcode::Neq;
        case BinaryOp::Exponentiation: return Opcode::Pow;
        default:
            throw CLException("Or/And shouldn't be here");
    }
}

Opcode opcode_from_unary(UnaryOp op) {
    switch(op) {
        case UnaryOp::Negation: return Opcode::Neg;
        case UnaryOp::Identity: return Opcode::Nop;
    }
}

    std::string SymbolTable::get_name(uint16_t name_index) const {
            if (name_index >= names.size() || name_index < 0)
                throw CLException("No such name with index " + std::to_string(name_index));
            return names[name_index];
    }

    LiteralValue SymbolTable::get_literal(uint32_t literal_index) const {
        if (literal_index >= literals.size() || literal_index < 0)
            throw CLException("No such name with index " + std::to_string(literal_index));
        return literals[literal_index];
    }
}
