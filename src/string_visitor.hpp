#pragma once

#include <string>

#include "nodes.hpp"
#include "stack_based_evaluator.hpp"

namespace Calculator {
class DebugPrinterEvaluator : public StackMachine<std::string>, public Evaluator {
private:
    uint8_t m_scope { 0 };
    std::string get_tabs() const noexcept;

public:
    std::string get_result() noexcept { return pop(); }

    void visit_number_expression(Number n) override;
    void visit_string_expression(String s) override;
    void visit_and_expression(const ExprPtr& left, const ExprPtr& right) override;
    void visit_or_expression(const ExprPtr& left, const ExprPtr& right) override;
    void visit_binary_expression(const ExprPtr& left, BinaryOp op, const ExprPtr& right) override;
    void visit_unary_expression(UnaryOp op, const ExprPtr& expr) override;
    void visit_var_expression(const std::string& var) override;
    void visit_assign_expression(const std::string& name, const ExprPtr& value) override;
    void visit_fun_call(const ExprPtr& fun, const ExprList& args) override;
    void visit_fun_def(const Names& names, const ExprPtr& body) override;
    void visit_block_expression(const ExprList& block) override;
    void visit_return_expression(const ExprPtr& expr) override;
    void visit_if_expression(const ExprPtr& cond, const ExprPtr& expr, const ExprPtr& else_branch) override;
    void visit_set_expression(const ExprPtr& obj, const ExprPtr& name, const ExprPtr& val) override;
    void visit_get_expression(const ExprPtr& obj, const ExprPtr& name) override;
};
}
