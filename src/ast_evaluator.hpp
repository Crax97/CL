#pragma once

#include "commons.hpp"
#include "environment.hpp"
#include "nodes.hpp"
#include "stack_based_evaluator.hpp"
#include "value.hpp"

namespace Calculator {
class ASTEvaluator : public StackMachine<RuntimeValue>, public Evaluator {
private:
    Env<RuntimeValue>& m_env;
    void visit_number_expression(Number n) override;

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

    void visit_if_expression(const ExprPtr& cond, const ExprPtr& expr, const ExprPtr& else_branch)
	override;

public:
    ASTEvaluator(Env<RuntimeValue>& env)
	: m_env(env)
    {
    }
    RuntimeValue run_expression(const ExprPtr& body);
    RuntimeValue get_result() { return pop(); }
};
class ASTFunction : public Callable {
private:
    ExprPtr m_body;
    Names m_arg_names;

public:
    ASTFunction(ExprPtr body, Names names)
	: m_body(body)
	, m_arg_names(names)
    {
    }
    RuntimeValue call(Args& args, Env<RuntimeValue>& env) override;
    uint8_t arity() override { return m_arg_names.size(); }
};

}
