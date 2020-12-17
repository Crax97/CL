//
// Created by Giovanni "Crax" Solimeno on 01/12/2020.
// This is CL's AST bytecode compiler
//

#pragma once

#include <set>
#include "nodes.hpp"
#include "stack_based_evaluator.hpp"

namespace CL {
    enum class Opcode : uint8_t {
        Nop                 = 0x00, // Does nothing, wasting one vm cycle
        Load_Literal        = 0x01, // push(literals[read32])
        Load                = 0x02, // push(memory[names[read16()]])
        Store               = 0x03, // memory[names[read16()]] = stack[0], pops stack
        Set                 = 0x04, // stack[-2][stack[-1]] = stack[0], pops stack, pushes stack[0]
        Get                 = 0x05, // push stack[-1][stack[0]], pops stack,
        List                = 0x06, // n -> The number of elements belonging to the list
        Dict                = 0x07, // m -> The number of associations belonging to the list
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
    std::string opcode_to_string(Opcode op);
    Opcode byte_to_opcode(uint8_t code);

    using OpcodeValue16 = uint16_t;
    using OpcodeValue32 = uint32_t;
    struct StackFrame {
        std::vector<uint8_t> bytecode;
        std::set<std::string> names;

        int add_opcode(Opcode op);
        int add_opcode(Opcode op, OpcodeValue16 value);
        int add_opcode32(Opcode op, OpcodeValue32 value);

        [[maybe_unused]] int set16(int position, OpcodeValue16 value);
        int set32(int position, OpcodeValue32 value);
        [[nodiscard]] int bytecode_count() const { return bytecode.size(); }
    };

    struct FunctionFrame : public StackFrame {
        [[maybe_unused]] std::vector<std::string> names;
        explicit FunctionFrame(std::vector<std::string> in_names) :
            names(std::move(in_names)){ }
    };

    using LiteralValue = std::variant<Number, String, std::shared_ptr<FunctionFrame>>;

    struct Program {
    public:
        [[maybe_unused]] std::shared_ptr<StackFrame> main;
        [[maybe_unused]] std::deque<std::string> names;
        [[maybe_unused]] std::deque<LiteralValue> literals;
    };

    class VMASTEvaluator : public Evaluator,
            public StackMachine<std::shared_ptr<StackFrame>> {
    private:
    protected:
        std::deque<std::string> names;
        std::deque<LiteralValue> literals;
    public:
        VMASTEvaluator() {
            push(std::make_unique<StackFrame>());
        }
        void visit_number_expression(Number n) override;
        void visit_string_expression(String s) override;
        void visit_dict_expression(const std::vector<std::pair<ExprPtr,
                ExprPtr>> &) override;
        void visit_list_expression(const ExprList &) override;
        void visit_and_expression(const ExprPtr &left,
                                          const ExprPtr &right) override;
        void visit_or_expression(const ExprPtr &left,
                                         const ExprPtr &right) override;
        void visit_binary_expression(const ExprPtr &left,
                                             BinaryOp op,
                                             const ExprPtr &right) override;
        void visit_unary_expression(UnaryOp op, const ExprPtr &expr) override;
        void visit_var_expression(const std::string &var) override;
        void visit_assign_expression(const std::string &name,
                                             const ExprPtr &value) override;
        void visit_fun_call(const ExprPtr &fun, const ExprList &args) override;
        void visit_fun_def(const Names &fun_names, const ExprPtr &body) override;
        void visit_block_expression(const ExprList &block) override;
        void visit_return_expression(const ExprPtr &expr) override;
        void visit_break_expression() override;
        void visit_continue_expression() override;
        void visit_if_expression(const ExprPtr &cond,
                                         const ExprPtr &expr,
                                         const ExprPtr &else_branch) override;
        void visit_while_expression(const ExprPtr &cond,
                                            const ExprPtr &body) override;
        void visit_for_expression(const std::string &name,
                                          const ExprPtr &iterator,
                                          const ExprPtr &body) override;
        void visit_set_expression(const ExprPtr &obj,
                                          const ExprPtr &what,
                                          const ExprPtr &value) override;
        void visit_get_expression(const ExprPtr &obj,
                                          const ExprPtr &what) override;
        void visit_module_definition(const ExprList &expressions) override;

        [[maybe_unused]] Program get_program() { return Program {peek(), names, literals}; }

        StackFrame& current_frame();
        uint32_t add_literal(const LiteralValue& v);
        uint16_t get_name_index(const std::string& name);
    };
}