//
// Created by Giovanni "Crax" Solimeno on 01/12/2020.
// This is CL's AST bytecode compiler
//

#pragma once

#include <set>
#include <utility>
#include "nodes.hpp"
#include "stack_based_evaluator.hpp"
#include "program.h"
#include "commons.hpp"

namespace CL {

    using OpcodeValue16 = uint16_t;
    using OpcodeValue32 = uint32_t;

    struct CompilationStackFrame {
        SymbolTablePtr symbol_table;
        std::vector<uint8_t> bytecode;
        int add_opcode(Opcode op);
        int add_opcode(Opcode op, OpcodeValue16 value);
        int add_opcode32(Opcode op, OpcodeValue32 value);
        int add_opcode8(Opcode op, uint8_t value);

        [[maybe_unused]] int set16(int position, OpcodeValue16 value);
        unsigned int set32(unsigned int position, OpcodeValue32 value);
        [[nodiscard]] int bytecode_count() const { return bytecode.size(); }
        [[nodiscard]] unsigned int current_offset() const { return bytecode.size() - 1; }

    };

    struct FunctionFrame : public CompilationStackFrame {
        [[maybe_unused]] std::vector<uint16_t> names;
        explicit FunctionFrame(std::vector<uint16_t> in_names) :
            names(std::move(in_names)){ }
    };

    class VMASTEvaluator : public Evaluator,
            public StackMachine<std::shared_ptr<CompilationStackFrame>> {
    private:
    protected:
        SymbolTablePtr symbol_table;
    public:
        explicit VMASTEvaluator(SymbolTablePtr in_symbol_table)
            : symbol_table(std::move(in_symbol_table)){
            push(std::make_unique<CompilationStackFrame>());
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
        void visit_return_expression(const ExprPtr &expr) override;
        void visit_break_expression() override;
        void visit_continue_expression() override;
        void visit_set_expression(const ExprPtr &obj,
                                          const ExprPtr &what,
                                          const ExprPtr &value) override;
        void visit_get_expression(const ExprPtr &obj,
                                          const ExprPtr &what) override;
        void visit_module_definition(const ExprList &expressions) override;

        void visit_fun_def_statement(const String& name, const Names &fun_names, const StatementPtr &body) override;
        void visit_block_statement(const StatementList &block) override;
        void visit_if_statement(const ExprPtr &cond,
                                const StatementPtr &expr,
                                const StatementPtr &else_branch) override;
        void visit_while_statement(const ExprPtr &cond,
                                   const StatementPtr &body) override;
        void visit_for_statement(const std::string &name,
                                 const ExprPtr &iterator,
                                 const StatementPtr &body) override;
        [[maybe_unused]] CompiledProgram get_program() { return CompiledProgram {peek(), symbol_table}; }

        CompilationStackFrame& current_frame();
        uint32_t add_literal(const LiteralValue& v);
        uint16_t get_name_index(const std::string& name);
    };
}