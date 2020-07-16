#include "ast_evaluator.hpp"
#include "commons.hpp"
#include "exceptions.hpp"
#include "value.hpp"
#include <algorithm>

namespace Calculator {
void ASTEvaluator::visit_number_expression(Number n)
{
    push(RuntimeValue(n));
}
void ASTEvaluator::visit_string_expression(String s)
{
    push(RuntimeValue(s));
}
void ASTEvaluator::visit_and_expression(const ExprPtr& left, const ExprPtr& right)
{
    left->evaluate(*this);
    auto l_result = pop();
    if (!l_result.is_truthy()) {
	push(RuntimeValue(0));
    } else {
	right->evaluate(*this);
    }
}
void ASTEvaluator::visit_or_expression(const ExprPtr& left, const ExprPtr& right)
{
    left->evaluate(*this);
    auto l_result = pop();
    if (l_result.is_truthy()) {
	push(RuntimeValue(1));
    } else {
	right->evaluate(*this);
    }
}
void ASTEvaluator::visit_binary_expression(const ExprPtr& left, BinaryOp op, const ExprPtr& right)
{
    right->evaluate(*this);
    left->evaluate(*this);
    auto l_val = pop();
    auto r_val = pop();

    switch (op) {
    case BinaryOp::Addition:
	push(l_val + r_val);
	break;
    case BinaryOp::Subtraction:
	push(l_val - r_val);
	break;
    case BinaryOp::Multiplication:
	push(l_val * r_val);
	break;
    case BinaryOp::Division:
	push(l_val / r_val);
	break;
    case BinaryOp::Exponentiation:
	push(l_val.to_power_of(r_val));
	break;
    case BinaryOp::Modulo:
	push(l_val.modulo(r_val));
	break;
    case BinaryOp::Less:
	push(l_val < r_val);
	break;
    case BinaryOp::Less_Equals:
	push(l_val <= r_val);
	break;
    case BinaryOp::Greater:
	push(l_val > r_val);
	break;
    case BinaryOp::Greater_Equals:
	push(l_val >= r_val);
	break;
    case BinaryOp::Equals:
	push(l_val == r_val);
	break;
    case BinaryOp::Not_Equals:
	push(l_val != r_val);
	break;
    case BinaryOp::And:
    case BinaryOp::Or:
	NOT_REACHED();
	break;
    }
}
void ASTEvaluator::visit_unary_expression(UnaryOp op, const ExprPtr& expr)
{
    expr->evaluate(*this);
    auto val = pop();

    switch (op) {
    case UnaryOp::Identity:
	push(val);
	break;
    case UnaryOp::Negation:
	val.negate();
	push(val);
	break;
    }
}
void ASTEvaluator::visit_var_expression(const std::string& var)
{
    push(m_env.get(var));
}
void ASTEvaluator::visit_assign_expression(const std::string& name, const ExprPtr& value)
{
    value->evaluate(*this);
    auto val = pop();
    m_env.assign(name, val);
    push(val);
}
void ASTEvaluator::visit_fun_call(const ExprPtr& fun, const ExprList& args)
{
    fun->evaluate(*this);
    auto call = pop();
    if (call.is_callable()) {
	auto callable = call.as_callable();

	if (args.size() != callable->arity() && callable->arity() != VAR_ARGS) {
	    throw RuntimeException(
		"This callable expects " + std::to_string(callable->arity())
		+ " arguments, but it got " + std::to_string(args.size()) + "!");
	}
	Args evaluated_args;
	for (const auto& arg : args) {
	    arg->evaluate(*this);
	    evaluated_args.push_back(pop());
	}
	m_env.scope_in();
	push(callable->call(evaluated_args, m_env));
	m_env.scope_out();
    } else {
	throw RuntimeException(call.to_string() + " is not callable.");
    }
}
void ASTEvaluator::visit_fun_def(const Names& names, const ExprPtr& body)
{
    auto fun = std::make_shared<ASTFunction>(body, names);
    auto val = RuntimeValue(fun);
    push(val);
}
void ASTEvaluator::visit_block_expression(const ExprList& block)
{
    std::for_each(block.begin(), block.end(), [this](const ExprPtr& expr) {
	expr->evaluate(*this);
    });
}

RuntimeValue ASTEvaluator::run_expression(const ExprPtr& expr)
{
    expr->evaluate(*this);
    return pop();
}
void ASTEvaluator::visit_return_expression(const ExprPtr& expr)
{
    TODO();
}
void ASTEvaluator::visit_set_expression(const ExprPtr& obj, const ExprPtr& name, const ExprPtr& val)
{
    obj->evaluate(*this);
    val->evaluate(*this);
    name->evaluate(*this);
    auto m_name = pop();
    auto m_val = pop();
    auto& m_obj = peek();

    m_obj.set_property(m_name, m_val);
    push(m_val);
}
void ASTEvaluator::visit_get_expression(const ExprPtr& obj, const ExprPtr& name)
{
    obj->evaluate(*this);
    name->evaluate(*this);
    auto m_name = pop();
    auto m_obj = pop();
    push(m_obj.get_property(m_name));
}

void ASTEvaluator::visit_if_expression(const ExprPtr& cond, const ExprPtr& if_branch, const ExprPtr& else_branch)
{
    cond->evaluate(*this);
    if (pop().is_truthy()) {
	if_branch->evaluate(*this);
    } else if (else_branch) {
	else_branch->evaluate(*this);
    }
}

RuntimeValue ASTFunction::call(Args& args, Env<RuntimeValue>& env)
{
    for (size_t i = 0; i < args.size(); i++) {
	env.assign(m_arg_names[i], args[i]);
    }
    ASTEvaluator evaluator(env);
    return evaluator.run_expression(m_body);
}

}
