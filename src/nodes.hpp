#pragma once

#include "commons.hpp"
#include "value.hpp"

#include <memory>
#include <string>
#include <variant>
#include <vector>
namespace Calculator {
class Evaluator {
public:
    virtual void visit_number_expression(Number n) = 0;
    virtual void visit_and_expression(const ExprPtr& left, const ExprPtr& right) = 0;
    virtual void visit_or_expression(const ExprPtr& left, const ExprPtr& right) = 0;
    virtual void visit_binary_expression(const ExprPtr& left, BinaryOp op, const ExprPtr& right) = 0;
    virtual void visit_unary_expression(UnaryOp op, const ExprPtr& expr) = 0;
    virtual void visit_var_expression(const std::string& var) = 0;
    virtual void visit_assign_expression(const std::string& name, const ExprPtr& value) = 0;
    virtual void visit_fun_call(const ExprPtr& fun, const ExprList& args) = 0;
    virtual void visit_fun_def(const Names& names, const ExprPtr& body) = 0;
    virtual void visit_block_expression(const ExprList& block) = 0;
    virtual void visit_return_expression(const ExprPtr& expr) = 0;
    virtual void visit_if_expression(const ExprPtr& cond, const ExprPtr& expr, const ExprPtr& else_branch) = 0;
};

class Expression {
public:
    virtual void evaluate(Evaluator& evaluator) const = 0;
};

class NumberExpression : public Expression {
private:
    Number m_val;

public:
    NumberExpression(Number val) noexcept
	: m_val(val)
    {
    }
    void evaluate(Evaluator& evaluator) const override { evaluator.visit_number_expression(m_val); }
};

class AndExpression : public Expression {
private:
    ExprPtr m_left;
    ExprPtr m_right;

public:
    AndExpression(ExprPtr left, ExprPtr right) noexcept
	: m_left(std::move(left))
	, m_right(std::move(right))
    {
    }
    void evaluate(Evaluator& evaluator) const override { evaluator.visit_and_expression(m_left, m_right); }
};

class OrExpression : public Expression {
private:
    ExprPtr m_left;
    ExprPtr m_right;

public:
    OrExpression(ExprPtr left, ExprPtr right) noexcept
	: m_left(std::move(left))
	, m_right(std::move(right))
    {
    }
    void evaluate(Evaluator& evaluator) const override { evaluator.visit_or_expression(m_left, m_right); }
};

class BinaryExpression : public Expression {
    ExprPtr m_left;
    ExprPtr m_right;
    BinaryOp m_op;

public:
    BinaryExpression(ExprPtr left, BinaryOp op, ExprPtr right) noexcept
	: m_left(std::move(left))
	, m_op(op)
	, m_right(std::move(right))
    {
    }
    void evaluate(Evaluator& evaluator) const override { evaluator.visit_binary_expression(m_left, m_op, m_right); }
};

class UnaryExpression : public Expression {
    ExprPtr m_expr;
    UnaryOp m_op;

public:
    UnaryExpression(ExprPtr expr, UnaryOp op) noexcept
	: m_expr(std::move(expr))
	, m_op(op)
    {
    }
    void evaluate(Evaluator& evaluator) const override
    {
	evaluator.visit_unary_expression(m_op, m_expr);
    }
};

class VarExpression : public Expression {
private:
    std::string m_name;

public:
    VarExpression(std::string name) noexcept
	: m_name(name)
    {
    }
    void evaluate(Evaluator& evaluator) const override { evaluator.visit_var_expression(m_name); }
};

class AssignExpression : public Expression {
private:
    std::string m_name;
    ExprPtr m_val;

public:
    AssignExpression(std::string name, ExprPtr val) noexcept
	: m_name(name)
	, m_val(std::move(val))
    {
    }
    void evaluate(Evaluator& evaluator) const override { evaluator.visit_assign_expression(m_name, m_val); }
};

class IfExpression : public Expression {
private:
    ExprPtr m_cond;
    ExprPtr m_body;
    ExprPtr m_else;

public:
    IfExpression(ExprPtr cond, ExprPtr body, ExprPtr else_branch)
	: m_cond(std::move(cond))
	, m_body(std::move(body))
	, m_else(std::move(else_branch))
    {
    }

    void evaluate(Evaluator& evaluator) const override { evaluator.visit_if_expression(m_cond, m_body, m_else); }
};

class FunCallExpression : public Expression {
private:
    ExprPtr m_expr;
    const ExprList m_args;

public:
    FunCallExpression(ExprPtr expr, ExprList args) noexcept
	: m_expr(std::move(expr))
	, m_args(std::move(args))
    {
    }
    void evaluate(Evaluator& evaluator) const override { evaluator.visit_fun_call(m_expr, m_args); }
};

class FunDef : public Expression {
private:
    const ExprPtr m_body;
    Names m_args;

public:
    FunDef(Names arg_names, ExprPtr body)
	: m_args(arg_names)
	, m_body(std::move(body))
    {
    }
    void evaluate(Evaluator& evaluator) const override { evaluator.visit_fun_def(m_args, m_body); }
};

class BlockExpression : public Expression {
private:
    ExprList m_body;

public:
    BlockExpression(ExprList body)
	: m_body(std::move(body))
    {
    }
    void evaluate(Evaluator& evaluator) const override { evaluator.visit_block_expression(m_body); }
};

class ReturnExpression : public Expression {
private:
    ExprPtr m_expr;

public:
    ReturnExpression(ExprPtr expr)
	: m_expr(std::move(expr))
    {
    }
    void evaluate(Evaluator& evaluator) const override { evaluator.visit_return_expression(m_expr); }
};

} // namespace Calculator
