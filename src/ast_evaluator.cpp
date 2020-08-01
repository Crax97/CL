#include "ast_evaluator.hpp"
#include "commons.hpp"
#include "environment.hpp"
#include "exceptions.hpp"
#include "value.hpp"

#include <algorithm>
#include <memory>

namespace CL {
void ASTEvaluator::visit_number_expression(Number n)
{
    push(n);
}
void ASTEvaluator::visit_string_expression(String s)
{
    push(s);
}

void ASTEvaluator::visit_dict_expression(const std::vector<std::pair<ExprPtr, ExprPtr>>& exprs)
{
    auto d = std::make_shared<Dictionary>();
    for (const auto& e : exprs) {
	e.first->evaluate(*this);
	e.second->evaluate(*this);

	auto r = pop();
	auto l = pop();
	d->set(l, r);
    }
    push(RuntimeValue(d));
}
void ASTEvaluator::visit_list_expression(const ExprList& exprs)
{
    auto l = std::make_shared<List>();
    for (const auto& e : exprs) {
	e->evaluate(*this);
	l->append(pop());
    }
    push(RuntimeValue(l));
}

void ASTEvaluator::visit_and_expression(const ExprPtr& left, const ExprPtr& right)
{
    left->evaluate(*this);
    auto l_result = pop();
    if (!l_result.is_truthy()) {
	push(RuntimeValue(false));
    } else {
	right->evaluate(*this);
    }
}
void ASTEvaluator::visit_or_expression(const ExprPtr& left, const ExprPtr& right)
{
    left->evaluate(*this);
    auto l_result = pop();
    if (l_result.is_truthy()) {
	push(RuntimeValue(true));
    } else {
	right->evaluate(*this);
    }
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"
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
	push(RuntimeValue((bool)(l_val < r_val)));
	break;
    case BinaryOp::Less_Equals:
	push(RuntimeValue((bool)(l_val <= r_val)));
	break;
    case BinaryOp::Greater:
	push(RuntimeValue((bool)(l_val > r_val)));
	break;
    case BinaryOp::Greater_Equals:
	push(RuntimeValue((bool)(l_val >= r_val)));
	break;
    case BinaryOp::Equals:
	push(RuntimeValue((bool)(l_val == r_val)));
	break;
    case BinaryOp::Not_Equals:
	push(RuntimeValue((bool)(l_val != r_val)));
	break;
    case BinaryOp::And:
    case BinaryOp::Or:
	NOT_REACHED()
	break;
    }
}
#pragma clang diagnostic pop
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
    push(m_env->get(var));
}
void ASTEvaluator::visit_assign_expression(const std::string& name, const ExprPtr& value)
{
    value->evaluate(*this);
    auto val = pop();
    m_env->assign(name, val, false);
    push(val);
}
void ASTEvaluator::visit_fun_call(const ExprPtr& fun, const ExprList& args)
{
    fun->evaluate(*this);
    auto call = pop();
    if (call.is<CallablePtr>()) {
	auto callable = call.as<CallablePtr>();

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
        auto result = callable->call(evaluated_args);
        if (result.has_value()) {
            push(result.value());
        }
    } else {
	    throw RuntimeException(call.to_string() + " is not callable.");
    }
}
void ASTEvaluator::visit_fun_def(const Names& names, const ExprPtr& body)
{
    auto fun = std::make_shared<ASTFunction>(body, names, m_env);
    auto val = RuntimeValue(fun);
    push(val);
}

    void ASTEvaluator::visit_return_expression(const ExprPtr& expr)
{
    expr->evaluate(*this);
    set_flag(FLAGS::RETURN);
}

void ASTEvaluator::visit_break_expression()
{
    set_flag(FLAGS::BREAK);
}

void ASTEvaluator::visit_continue_expression()
{
    set_flag(FLAGS::CONTINUE);
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

void ASTEvaluator::visit_while_expression(const ExprPtr& cond, const ExprPtr& body)
{
    cond->evaluate(*this);
    while (pop().is_truthy()) {
	if (!is_flag_set(FLAGS::CONTINUE))
	    body->evaluate(*this);
	if (is_flag_set(FLAGS::RETURN) || is_flag_set(FLAGS::BREAK))
	    break;
	cond->evaluate(*this);
    }
}

void ASTEvaluator::visit_for_expression(const std::string& name, const ExprPtr& iterable, const ExprPtr& body)
{
    iterable->evaluate(*this);
    auto iterable_val = pop();

    auto has_next_fun = iterable_val.get_named("__has_next").as<CallablePtr>();
    auto next_fun = iterable_val.get_named("__next").as<CallablePtr>();
    while (has_next_fun->call().value().is_truthy()) {
	auto val = next_fun->call().value();
	m_env->assign(name, val, false);
	if (!is_flag_set(FLAGS::CONTINUE))
	    body->evaluate(*this);
	if (is_flag_set(FLAGS::RETURN) || is_flag_set(FLAGS::BREAK))
	    break;
    }
}

void ASTEvaluator::visit_block_expression(const ExprList& block)
{
    auto env = std::make_shared<StackedEnvironment>(m_env);
    auto old_env = m_env;
    m_env = env;
    for (const auto& expr : block) {
	expr->evaluate(*this);
	if (is_any_flag_set())
	    break;
    }
    m_env = old_env;
}

void ASTEvaluator::visit_module_definition(const ExprList& list)
{
    auto env = std::make_shared<StackedEnvironment>(m_env);
    auto evaluator = ASTEvaluator(env);
    for (auto& expr : list) {
        expr->evaluate(evaluator);
    }
    auto mod = std::make_shared<Module>(env);
    push(std::dynamic_pointer_cast<Indexable>(mod));
}

std::optional<RuntimeValue> ASTFunction::call(const Args& args)
{
    auto env = std::make_shared<StackedEnvironment>(m_definition_env);
    for (size_t i = 0; i < args.size(); i++) {
	env->assign(m_arg_names[i], args[i], false);
    }
    ASTEvaluator evaluator(env);
    m_body->evaluate(evaluator);
	return evaluator.get_result();
}
}
