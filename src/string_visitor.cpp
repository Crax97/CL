#include "string_visitor.hpp"
#include <algorithm>
#include <sstream>

namespace Calculator {
void DebugPrinterEvaluator::visit_number_expression(Number n)
{
    auto str_repr_of_number = std::to_string(n);
    auto last_index_of_dot = str_repr_of_number.find_last_not_of('0') + 1;
    str_repr_of_number.erase(last_index_of_dot, str_repr_of_number.size());
    if (str_repr_of_number[str_repr_of_number.size() - 1] == '.') {
	str_repr_of_number.pop_back();
    }
    push(str_repr_of_number);
}
void DebugPrinterEvaluator::visit_string_expression(String s)
{
    push("\"" + s + "\"");
}
void DebugPrinterEvaluator::visit_dict_expression(const std::vector<std::pair<ExprPtr, ExprPtr>>& exprs)
{
    std::stringstream str;
    str << "dict {\n";
    for (const auto& e : exprs) {
	e.first->evaluate(*this);
	e.second->evaluate(*this);
	auto right = pop();
	auto left = pop();
	str << "\t" << left << " : " << right << "\n";
    }
    str << "}";
    push(str.str());
}

void DebugPrinterEvaluator::visit_list_expression(const ExprList& exprs)
{
    std::stringstream str;
    str << "list (\n";
    for (const auto& e : exprs) {
	e->evaluate(*this);
	str << pop() << ", ";
    }
    str << ")";
    push(str.str());
}

void DebugPrinterEvaluator::visit_and_expression(const ExprPtr& left, const ExprPtr& right)
{
    left->evaluate(*this);
    right->evaluate(*this);
    push(pop() + " and " + pop());
}
void DebugPrinterEvaluator::visit_or_expression(const ExprPtr& left, const ExprPtr& right)
{
    left->evaluate(*this);
    right->evaluate(*this);
    push(pop() + " or " + pop());
}
void DebugPrinterEvaluator::visit_binary_expression(const ExprPtr& left, BinaryOp op, const ExprPtr& right)
{
    left->evaluate(*this);
    right->evaluate(*this);
    push(pop() + " " + binary_op_to_string(op) + " " + pop());
}
void DebugPrinterEvaluator::visit_unary_expression(UnaryOp op, const ExprPtr& expr)
{
    expr->evaluate(*this);
    push(unary_op_to_string(op) + pop());
}
void DebugPrinterEvaluator::visit_var_expression(const std::string& var)
{
    push(var);
}
void DebugPrinterEvaluator::visit_assign_expression(const std::string& name, const ExprPtr& value)
{
    value->evaluate(*this);
    push(name + " = " + pop());
}
void DebugPrinterEvaluator::visit_fun_call(const ExprPtr& fun, const ExprList& args)
{
    std::string arg_str = " ";
    std::for_each(args.begin(), args.end(), [&arg_str, this](const auto& ex) {
	ex->evaluate(*this);
	arg_str.append(pop() + " ");
    });
    fun->evaluate(*this);
    push(pop() + "(" + arg_str + ")");
}
void DebugPrinterEvaluator::visit_fun_def(const Names& names, const ExprPtr& body)
{
    m_scope++;
    body->evaluate(*this);

    std::string name_string = " ";
    std::for_each(names.begin(), names.end(), [&name_string](const auto& name) { name_string.append(name + " "); });
    push("(" + name_string + ") = " + pop());
    m_scope--;
}
void DebugPrinterEvaluator::visit_block_expression(const ExprList& block)
{
    std::string result;
    std::for_each(block.begin(), block.end(), [&result, this](const auto& expr) {
	expr->evaluate(*this);
	result.append(pop());
    });
    push(get_tabs() + "{\n " + result + "\n}");
}
void DebugPrinterEvaluator::visit_return_expression(const ExprPtr& expr)
{
    expr->evaluate(*this);
    push("return " + pop());
}
void DebugPrinterEvaluator::visit_break_expression()
{
    push("break");
}
void DebugPrinterEvaluator::visit_continue_expression()
{
    push("continue");
}
void DebugPrinterEvaluator::visit_if_expression(const ExprPtr& cond, const ExprPtr& expr, const ExprPtr& else_branch)
{

    expr->evaluate(*this);
    cond->evaluate(*this);

    std::string result = pop() + " if " + pop();
    if (else_branch) {
	else_branch->evaluate(*this);
	auto else_str = pop();
	result += "\n else " + else_str;
    }
    push(result);
}

void DebugPrinterEvaluator::visit_while_expression(const ExprPtr& cond, const ExprPtr& body)
{
    cond->evaluate(*this);
    body->evaluate(*this);

    auto body_str = pop();

    push("while " + pop() + " " + body_str);
}
void DebugPrinterEvaluator::visit_for_expression(const std::string& name, const ExprPtr& iterable, const ExprPtr& body)
{
    iterable->evaluate(*this);
    body->evaluate(*this);

    auto body_str = pop();
    auto iterable_str = pop();

    push("for " + name + " in " + iterable_str + " " + body_str);
}
void DebugPrinterEvaluator::visit_set_expression(const ExprPtr& obj, const ExprPtr& name, const ExprPtr& val)
{
    obj->evaluate(*this);
    name->evaluate(*this);
    val->evaluate(*this);
    push(pop() + "[" + pop() + "] = " + pop());
}
void DebugPrinterEvaluator::visit_get_expression(const ExprPtr& obj, const ExprPtr& name)
{
    obj->evaluate(*this);
    name->evaluate(*this);
    push(pop() + "[" + pop() + "]");
}

void DebugPrinterEvaluator::visit_module_definition(const ExprList& list)
{
    std::stringstream str;
    str << "module {\n";
    for (const auto& expr : list) {
	expr->evaluate(*this);
	str << "\t" << pop();
    }
    str << "\n}";
    push(str.str());
}
std::string DebugPrinterEvaluator::get_tabs() const noexcept
{
    return std::string(m_scope, '\t');
}
}
