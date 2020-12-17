//
// Created by Giovanni "Crax" Solimeno on 01/12/2020.
//

#include "vm_ast_evaluator.h"
#include "commons.hpp"


class [[maybe_unused]] CompilationException : public CL::CLException {
public:
    [[maybe_unused]] explicit CompilationException(const std::string &error_message) :
        CL::CLException("Compilation error: " + error_message) {}
};

namespace CL {

    void VMASTEvaluator::visit_number_expression(CL::Number n) {
        uint32_t index = add_literal(n);
        current_frame().add_opcode32(Opcode::Load_Literal, index);
    }

    void VMASTEvaluator::visit_string_expression(String s) {
        uint32_t index = add_literal(s);
        current_frame().add_opcode32(Opcode::Load_Literal, index);
    }

    void VMASTEvaluator::visit_list_expression(const ExprList &List) {
        for(auto& expr : List) {
            expr->evaluate(*this);
        }
        current_frame().add_opcode32(Opcode::List, List.size());
    }

    void VMASTEvaluator::visit_dict_expression(const std::vector<std::pair<ExprPtr, ExprPtr>> &dict_expressions) {
        for(auto& pair : dict_expressions) {
            pair.first->evaluate(*this);
            pair.second->evaluate(*this);
        }
        current_frame().add_opcode32(Opcode::Dict, dict_expressions.size());
    }

    void VMASTEvaluator::visit_and_expression(const ExprPtr &left, const ExprPtr &right) {
        right->evaluate(*this);
        int jump_position = current_frame().add_opcode32(Opcode::Jump_False, 0);
        left->evaluate(*this);
        current_frame().set32(jump_position, current_frame().bytecode_count());
    }
    void VMASTEvaluator::visit_or_expression(const ExprPtr &left, const ExprPtr &right) {
        right->evaluate(*this);
        int jump_position = current_frame().add_opcode32(Opcode::Jump_True, 0);
        left->evaluate(*this);
        current_frame().set32(jump_position, current_frame().bytecode_count());
    }

    void VMASTEvaluator::visit_binary_expression(const ExprPtr &left, BinaryOp op, const ExprPtr &right) {
        right->evaluate(*this);
        left->evaluate(*this);
        current_frame().add_opcode(opcode_from_binary(op));
    }
    void VMASTEvaluator::visit_unary_expression(UnaryOp op, const ExprPtr &expr) {
        expr->evaluate(*this);
        current_frame().add_opcode(opcode_from_unary(op));
    }

    void VMASTEvaluator::visit_var_expression(const std::string &var) {
        int name_index = get_name_index(var);
        current_frame().add_opcode(Opcode::Load, name_index);
    }

    void VMASTEvaluator::visit_assign_expression(const std::string &name, const ExprPtr &value) {
        uint16_t name_index = get_name_index(name);
        value->evaluate(*this);
        current_frame().add_opcode(Opcode::Store, name_index);

    }

    void VMASTEvaluator::visit_fun_call(const ExprPtr &fun, const ExprList &args) {
        for (auto& arg : args) {
            arg->evaluate(*this);
        }
        fun->evaluate(*this);
        current_frame().add_opcode8(Opcode::Call, args.size());
    }

    void VMASTEvaluator::visit_fun_def(const Names &fun_names, const ExprPtr &body) {
        std::vector<uint16_t> indices;
        indices.reserve(fun_names.size());
        for (auto& name : fun_names) {
            indices.push_back(get_name_index(name));
        }
        auto function = std::make_shared<FunctionFrame>(indices);
        push(function);
        body->evaluate(*this);
        pop();

        int32_t index = add_literal(function);
        current_frame().add_opcode32(Opcode::Load_Literal, index);
    }

    void VMASTEvaluator::visit_block_expression(const ExprList &block) {
        current_frame().add_opcode(Opcode::Push_Frame);
        for(auto& expr : block) {
            expr->evaluate(*this);
        }
        current_frame().add_opcode(Opcode::Pop_Frame);
    }

    void VMASTEvaluator::visit_return_expression(const ExprPtr &expr) {
        if (expr != nullptr) {
            expr->evaluate(*this);
            current_frame().add_opcode(Opcode::Return);
        } else {
            current_frame().add_opcode(Opcode::Break);
        }
    }

    void VMASTEvaluator::visit_break_expression() {
        current_frame().add_opcode(Opcode::Break);
    }

    void VMASTEvaluator::visit_continue_expression() {
        current_frame().add_opcode(Opcode::Continue);
    }

    void VMASTEvaluator::visit_if_expression(const ExprPtr &cond, const ExprPtr &expr, const ExprPtr &else_branch) {
        cond->evaluate(*this);
        int jump_else_location = current_frame().add_opcode32(Opcode::Jump_False, 0);
        expr->evaluate(*this);
        int jump_if_location = current_frame().add_opcode32(Opcode::Jump, 0);
        current_frame().set32(jump_else_location, current_frame().bytecode_count());
        else_branch->evaluate(*this);
        current_frame().set32(jump_if_location, current_frame().bytecode_count());
    }

    void VMASTEvaluator::visit_while_expression(const ExprPtr &cond, const ExprPtr &body) {
        cond->evaluate(*this);
        int jump_location = current_frame().add_opcode32(Opcode::Jump_False, 0);
        body->evaluate(*this);
        current_frame().set32(jump_location, current_frame().bytecode_count());
    }

    void VMASTEvaluator::visit_for_expression(const std::string &name, const ExprPtr &iterator, const ExprPtr &body) {
        get_name_index(name);
        iterator->evaluate(*this);
        current_frame().add_opcode(Opcode::Get_Iter);

        current_frame().add_opcode(Opcode::Iter_Has_Next);
        int for_location = current_frame().bytecode_count();
        current_frame().add_opcode32(Opcode::Jump_False, 0);
        int jump_false_location = current_frame().bytecode_count();
        current_frame().add_opcode(Opcode::Get_Iter_Next);
        body->evaluate(*this);
        current_frame().add_opcode32(Opcode::Jump, for_location);

        current_frame().set32(jump_false_location, current_frame().bytecode_count());
    }

    void VMASTEvaluator::visit_set_expression(const ExprPtr &obj, const ExprPtr &what, const ExprPtr &value) {
        obj->evaluate(*this);
        what->evaluate(*this);
        value->evaluate(*this);
        current_frame().add_opcode(Opcode::Set);
    }

    void VMASTEvaluator::visit_get_expression(const ExprPtr &obj, const ExprPtr &what) {
        obj->evaluate(*this);
        what->evaluate(*this);
        current_frame().add_opcode(Opcode::Get);
    }

    void VMASTEvaluator::visit_module_definition(const ExprList &expressions) {
        current_frame().add_opcode32(Opcode::Module, expressions.size());
        for (auto& expr : expressions) {
            expr->evaluate(*this);
        }
    }

    CompilationStackFrame &VMASTEvaluator::current_frame() {
        return *peek();
    }


    uint32_t VMASTEvaluator::add_literal(const LiteralValue& v) {
        auto literal_found = std::find(literals.begin(), literals.end(), v);
        if (literal_found != literals.end()) {
            return std::distance(literals.begin(), literal_found);
        }
        literals.push_back(v);
        return literals.size() - 1;
    }

    uint16_t VMASTEvaluator::get_name_index(const std::string &name) {
        auto name_found = std::find(names.begin(), names.end(), name);
        if (name_found != names.end()) {
            return std::distance(names.begin(), name_found);
        }
        names.push_back(name);
        return names.size() - 1;
    }

    int CompilationStackFrame::add_opcode(Opcode op) {
        bytecode.push_back(static_cast<uint8_t>(op));
        return bytecode_count() - 1;
    }

    int CompilationStackFrame::add_opcode8(Opcode op, uint8_t value) {
        int position = bytecode_count();
        bytecode.push_back((uint8_t)(op));
        bytecode.push_back((uint8_t)(value));
        return position;
    }

    int CompilationStackFrame::add_opcode(Opcode op, OpcodeValue16 value) {
        int position = bytecode_count();
        bytecode.push_back((uint8_t)(op));
        bytecode.push_back((uint8_t)(value >> 8));
        bytecode.push_back((uint8_t)(value & 0x00FF));
        return position;
    }

    int CompilationStackFrame::add_opcode32(Opcode op, OpcodeValue32 value) {
        int position = bytecode_count();
        bytecode.push_back((uint8_t)(op));
        bytecode.push_back((uint8_t)(value >> 24));
        bytecode.push_back((uint8_t)(value >> 16));
        bytecode.push_back((uint8_t)(value >> 8));
        bytecode.push_back((uint8_t)(value & 0x00FF));
        return position;
    }

    [[maybe_unused]] int CompilationStackFrame::set16(int position, OpcodeValue16 value) {
        uint8_t higher = value >> 8;
        uint8_t lower = value & 0xFF;
        bytecode[position] = higher;
        bytecode[position + 1] = lower;
        return position;
    }

    int CompilationStackFrame::set32(int position, OpcodeValue32 value) {
        uint8_t byte1 = value >> 24;
        uint8_t byte2 = value >> 16;
        uint8_t byte3 = value >> 8;
        uint8_t byte4 = value & 0xFF;
        bytecode[position + 0] = byte1;
        bytecode[position + 1] = byte2;
        bytecode[position + 2] = byte3;
        bytecode[position + 3] = byte4;
        return position;
    }

    std::string opcode_to_string(Opcode op) {
        switch (op) {

            case Opcode::Nop: return "Nop";
            case Opcode::Load_Literal: return "Load Literal";
            case Opcode::Load: return "Load";
            case Opcode::Store: return "Store";
            case Opcode::Set: return "Set";
            case Opcode::Get: return "Get";
            case Opcode::List: return "List";
            case Opcode::Dict: return "Dict";
            case Opcode::Neg: return "Neg";
            case Opcode::Add: return "Add";
            case Opcode::Sub: return "Sub";
            case Opcode::Mul: return "Mul";
            case Opcode::Div: return "Div";
            case Opcode::Mod: return "Mod";
            case Opcode::Pow: return "Pow";
            case Opcode::Less: return "Less";
            case Opcode::Less_Eq: return "Less Equals";
            case Opcode::Greater: return "Greater";
            case Opcode::Greater_Eq: return "Greater Equals";
            case Opcode::Eq: return "Eq";
            case Opcode::Neq: return "Neq";
            case Opcode::True: return "Is True";
            case Opcode::Push_Frame: return "Push Frame";
            case Opcode::Pop_Frame: return "Pop Frame";
            case Opcode::Jump_True: return "Jump If True";
            case Opcode::Jump_False: return "Jump If False";
            case Opcode::Jump: return "Jump";
            case Opcode::Call: return "Call";
            case Opcode::Module: return "Module";
            case Opcode::Return: return "Return";
            case Opcode::Break: return "Break";
            case Opcode::Continue: return "Continue";
            case Opcode::Get_Iter: return "Get Iter";
            case Opcode::Iter_Has_Next: return "Iter Has Next";
            case Opcode::Get_Iter_Next: return "Get Iter Next";
        }
    }

    Opcode byte_to_opcode(uint8_t code) {
        return static_cast<Opcode>(code);
    }
}
