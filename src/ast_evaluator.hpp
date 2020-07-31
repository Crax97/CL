#pragma once

#include <utility>

#include "commons.hpp"
#include "environment.hpp"
#include "nodes.hpp"
#include "stack_based_evaluator.hpp"
#include "string_visitor.hpp"
#include "value.hpp"

namespace Calculator {
class ASTEvaluator : public StackMachine<RuntimeValue>, public Evaluator {
private:
    enum class FLAGS {
	NONE = 0x00,
	RETURN = 0x01,
	CONTINUE = 0x02,
	BREAK = 0x04,

    };
    std::shared_ptr<StackedEnvironment> m_env;
    FLAGS m_flags = FLAGS::NONE;

    bool is_flag_set(FLAGS flag)
    {
	if (m_flags == flag) {
	    m_flags = FLAGS::NONE;
	    return true;
	}
	return false;
    }

    bool is_any_flag_set() { return m_flags != FLAGS::NONE; }
    void set_flag(FLAGS flag)
    {
	m_flags = flag;
    }

    void visit_number_expression(Number n) override;
    void visit_string_expression(String s) override;
    void visit_dict_expression(const std::vector<std::pair<ExprPtr, ExprPtr>>&) override;
    void visit_list_expression(const ExprList&) override;

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
    void visit_break_expression() override;
    void visit_continue_expression() override;
    void visit_if_expression(const ExprPtr& cond, const ExprPtr& expr, const ExprPtr& else_branch)
	override;
    void visit_while_expression(const ExprPtr& cond, const ExprPtr& body) override;
    void visit_for_expression(const std::string& name, const ExprPtr& iterable, const ExprPtr& body) override;
    void visit_set_expression(const ExprPtr& obj, const ExprPtr& name, const ExprPtr& val) override;
    void visit_get_expression(const ExprPtr& obj, const ExprPtr& name) override;
    void visit_module_definition(const ExprList& list) override;

public:
    explicit ASTEvaluator(std::shared_ptr<StackedEnvironment> env)
	: m_env(std::move(env))
    {
    }

    std::optional<RuntimeValue> get_result() {
        if (!m_stack.empty())
            return pop();
        return std::nullopt;
    }
};
class ASTFunction : public Callable {
private:
    ExprPtr m_body;
    std::shared_ptr<StackedEnvironment> m_definition_env;
    Names m_arg_names;

public:
    ASTFunction(ExprPtr body, Names names, std::shared_ptr<StackedEnvironment> definition_env)
	: m_body(std::move(body))
	, m_arg_names(std::move(names))
	, m_definition_env(std::move(definition_env))
    {
    }
    std::optional<RuntimeValue> call(const Args& args) override;
    uint8_t arity() override { return m_arg_names.size(); }
    [[nodiscard]]
    std::string string_repr() const noexcept override
    {
	StringVisitor eval;
	std::string name_string;
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
