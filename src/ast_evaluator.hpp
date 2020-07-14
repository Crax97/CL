#pragma once

#include "commons.hpp"
#include "environment.hpp"
#include "nodes.hpp"
#include "stack_based_evaluator.hpp"
#include "string_visitor.hpp"
#include "value.hpp"

namespace Calculator {
class ASTEvaluator : public StackMachine<RuntimeValue>, public Evaluator {
private:
    Env<RuntimeValue>& m_env;
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

    void visit_if_expression(const ExprPtr& cond, const ExprPtr& expr, const ExprPtr& else_branch)
	override;
    void visit_set_expression(const ExprPtr& obj, const std::string& name, const ExprPtr& val) override;
    void visit_get_expression(const ExprPtr& obj, const std::string& name) override;

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
    std::string string_repr() const noexcept override
    {
	DebugPrinterEvaluator eval;
	std::string name_string = "";
	for (const auto& name : m_arg_names) {
	    name_string += name + ", ";
	}

	if (name_string.length() > 2) {
	    name_string.pop_back();
	    name_string.pop_back();
	}
	m_body->evaluate(eval);
	return "fun( " + name_string + " ) -> " + eval.get_result();
    }
};

}
