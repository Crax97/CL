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
        Nop, // Does nothing, wasting one vm cycle
        Load_Literal, // push(literals[read32])
        Load, // push(memory[names[read16()]])
        Store, // memory[names[read16()]] = stack[0], pops stack
        Set, // stack[-2][stack[-1]] = stack[0], pops stack, pushes stack[0]
        Get, // push stack[-1][stack[0]], pops stack,
        List, // n -> The number of elements belonging to the list
        Dict, // m -> The number of associations belonging to the list
        Neg, // push(-stack[0]), pops stack
        Add, // push(stack[0] + stack[-1]), pops stack
        Sub, // push(stack[0] - stack[-1]), pops stack
        Mul, // push(stack[0] * stack[-1]), pops stack
        Div, // push(stack[0] / stack[-1]), pops stack
        Mod, // push(stack[0] % stack[-1]), pops stack
        Pow, // push(stack[0] ^ stack[-1]), pops stack
        Less, // push(stack[0] < stack[-1]), pops stack
        Less_Eq, // push(stack[0] <= stack[-1]), pops stack
        Greater, // push(stack[0] > stack[-1]), pops stack
        Greater_Eq, // push(stack[0] >= stack[-1]), pops stack
        Eq, // push(stack[0] == stack[-1]), pops stack
        Neq, // push(stack[0] != stack[-1]), pops stack
        True [[maybe_unused]], // push(truthy(stack[0])), pops stack
        Push_Frame, // push new stack frame
        Pop_Frame, // pops last stack frame
        Jump_True, // Jumps to read32() if true
        Jump_False, // Jumps to read32() if false
        Jump, // Jumps to read32()
        Call, // k -> number of arguments, stack[0](arg_1...arg_k) where arg_i is the result of the i-eth expression after the current opcode
        Module, // k -> the next k expressions belong to the module
        Return, // stack_frames[-1].return_value = stack[0]; stack_frames.pop();
        Break, // stack_frames.pop();
        Continue, // stack_frames.pop();
        Get_Iter, // push(get_iterator(stack[0])), pops stack[0]
        Iter_Has_Next, // push(has_next(stack[0])), pops nothing
        Get_Iter_Next // push(get_next(stack[0])), pops nothing
    };
    using OpcodeValue16 = uint16_t;
    using OpcodeValue32 = uint32_t;
    struct StackFrame {
        std::vector<uint8_t> bytecode;
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
        [[maybe_unused]] std::vector<std::shared_ptr<FunctionFrame>> functions;
        [[maybe_unused]] std::set<std::string> names;
        [[maybe_unused]] std::set<LiteralValue> literals;
    };

    class VMASTEvaluator : public Evaluator,
            public StackMachine<std::shared_ptr<StackFrame>> {
    private:
    protected:
        std::vector<std::shared_ptr<FunctionFrame>> functions;
        std::set<std::string> names;
        std::set<LiteralValue> literals;
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

        [[maybe_unused]] Program get_program() { return Program {peek(), functions, names, literals}; }

        StackFrame& current_frame();
        uint32_t add_literal(const LiteralValue& v);
        uint16_t add_name(const std::string& name);
        int get_name_index(const std::string& name);
    };
}