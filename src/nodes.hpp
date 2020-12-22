#pragma once

#include "commons.hpp"
#include "value.hpp"

#include <memory>
#include <string>
#include <utility>
#include <utility>
#include <variant>
#include <vector>
namespace CL {
class Evaluator {
public:
	virtual void visit_number_expression(Number n) = 0;
	virtual void visit_string_expression(String s) = 0;
	virtual void visit_dict_expression(const std::vector<std::pair<ExprPtr,
																   ExprPtr>> &) = 0;
	virtual void visit_list_expression(const ExprList &) = 0;
	virtual void visit_and_expression(const ExprPtr &left,
									  const ExprPtr &right) = 0;
	virtual void visit_or_expression(const ExprPtr &left,
									 const ExprPtr &right) = 0;
	virtual void visit_binary_expression(const ExprPtr &left,
										 BinaryOp op,
										 const ExprPtr &right) = 0;
	virtual void visit_unary_expression(UnaryOp op, const ExprPtr &expr) = 0;
	virtual void visit_var_expression(const std::string &var) = 0;
	virtual void visit_assign_expression(const std::string &name,
										 const ExprPtr &value) = 0;
	virtual void visit_fun_call(const ExprPtr &fun, const ExprList &args) = 0;
	virtual void visit_fun_def_statement(const String& name, const Names &names, const StatementPtr &body) = 0;
	virtual void visit_block_statement(const StatementList &block) = 0;
	virtual void visit_return_expression(const ExprPtr &expr) = 0;
	virtual void visit_break_expression() = 0;
	virtual void visit_continue_expression() = 0;
	virtual void visit_if_statement(const ExprPtr &cond,
                                    const StatementPtr &expr,
                                    const StatementPtr &else_branch) = 0;
	virtual void visit_while_statement(const ExprPtr &cond,
                                       const StatementPtr &body) = 0;
	virtual void visit_for_statement(const std::string &name,
                                     const ExprPtr &iterator,
                                     const StatementPtr &body) = 0;
	virtual void visit_set_expression(const ExprPtr &obj,
									  const ExprPtr &what,
									  const ExprPtr &value) = 0;
	virtual void visit_get_expression(const ExprPtr &obj,
									  const ExprPtr &what) = 0;
	virtual void visit_module_definition(const ExprList &exprs) = 0;
};

class Expression {
public:
	virtual void evaluate(Evaluator &evaluator) const = 0;
};

class Statement {
public:
    virtual void execute(Evaluator& evaluator) const = 0;
};

class NumberExpression : public Expression {
private:
	Number m_val;

public:
	explicit NumberExpression(Number val) noexcept
		: m_val(val) {
	}
	void evaluate(Evaluator &evaluator) const override {
		evaluator.visit_number_expression(m_val);
	}
};

class StringExpression : public Expression {
private:
	String m_str;

public:
	explicit StringExpression(String s) noexcept
		: m_str(std::move(s)) {
	}
	void evaluate(Evaluator &evaluator) const override {
		evaluator.visit_string_expression(m_str);
	}
};

class DictExpression : public Expression {
private:
	std::vector<std::pair<ExprPtr, ExprPtr>> m_exprs;

public:
	explicit DictExpression(std::vector<std::pair<ExprPtr, ExprPtr>> exprs)
		: m_exprs(std::move(std::move(exprs))) {
	}
	void evaluate(Evaluator &evaluator) const override {
		evaluator.visit_dict_expression(m_exprs);
	}
};

class ListExpression : public Expression {
private:
	ExprList m_exprs;

public:
	explicit ListExpression(ExprList exprs)
		: m_exprs(std::move(std::move(exprs))) {
	}
	void evaluate(Evaluator &evaluator) const override {
		evaluator.visit_list_expression(m_exprs);
	}
};

class AndExpression : public Expression {
private:
	ExprPtr m_left;
	ExprPtr m_right;

public:
	explicit AndExpression(ExprPtr left, ExprPtr right) noexcept
		: m_left(std::move(left)), m_right(std::move(right)) {
	}
	void evaluate(Evaluator &evaluator) const override {
		evaluator.visit_and_expression(m_left,
									   m_right);
	}
};

class OrExpression : public Expression {
private:
	ExprPtr m_left;
	ExprPtr m_right;

public:
	explicit OrExpression(ExprPtr left, ExprPtr right) noexcept
		: m_left(std::move(left)), m_right(std::move(right)) {
	}
	void evaluate(Evaluator &evaluator) const override {
		evaluator.visit_or_expression(m_left,
									  m_right);
	}
};

class BinaryExpression : public Expression {
	ExprPtr m_left;
	ExprPtr m_right;
	BinaryOp m_op;

public:
	explicit BinaryExpression(ExprPtr left, BinaryOp op, ExprPtr right) noexcept
		: m_left(std::move(left)), m_op(op), m_right(std::move(right)) {
	}
	void evaluate(Evaluator &evaluator) const override {
		evaluator.visit_binary_expression(m_left,
										  m_op,
										  m_right);
	}
};

class UnaryExpression : public Expression {
	ExprPtr m_expr;
	UnaryOp m_op;

public:
	explicit UnaryExpression(ExprPtr expr, UnaryOp op) noexcept
		: m_expr(std::move(expr)), m_op(op) {
	}
	void evaluate(Evaluator &evaluator) const override {
		evaluator.visit_unary_expression(m_op, m_expr);
	}
};

class VarExpression : public Expression {
private:
	std::string m_name;

public:
	explicit VarExpression(std::string name) noexcept
		: m_name(std::move(name)) {
	}
	void evaluate(Evaluator &evaluator) const override {
		evaluator.visit_var_expression(m_name);
	}
};

class AssignExpression : public Expression {
private:
	std::string m_name;
	ExprPtr m_val;

public:
	explicit AssignExpression(std::string name, ExprPtr val) noexcept
		: m_name(std::move(name)), m_val(std::move(val)) {
	}
	void evaluate(Evaluator &evaluator) const override {
		evaluator.visit_assign_expression(m_name,
										  m_val);
	}
};

class FunCallExpression : public Expression {
    private:
        ExprPtr m_expr;
        const ExprList m_args;

    public:
        explicit FunCallExpression(ExprPtr expr, ExprList args) noexcept
                : m_expr(std::move(expr)), m_args(std::move(args)) {
        }
        void evaluate(Evaluator &evaluator) const override {
            evaluator.visit_fun_call(m_expr,
                                     m_args);
        }
    };

class ReturnExpression : public Expression {
private:
	ExprPtr m_expr;

public:
	explicit ReturnExpression(ExprPtr expr)
		: m_expr(std::move(expr)) {
	}
	void evaluate(Evaluator &evaluator) const override {
		evaluator.visit_return_expression(m_expr);
	}
};

class BreakExpression : public Expression {
public:
	void evaluate(Evaluator &evaluator) const override { evaluator.visit_break_expression(); }
};

class ContinueExpression : public Expression {
public:
	void evaluate(Evaluator &evaluator) const override { evaluator.visit_continue_expression(); }
};

class SetExpression : public Expression {
private:
	ExprPtr m_obj;
	ExprPtr m_name;
	ExprPtr m_val;

public:
	SetExpression(ExprPtr obj, ExprPtr name, ExprPtr val)
		: m_obj(std::move(obj)),
		  m_name(std::move(name)),
		  m_val(std::move(val)) {
	}
	void evaluate(Evaluator &evaluator) const override {
		evaluator.visit_set_expression(m_obj,
									   m_name,
									   m_val);
	}
};
class GetExpression : public Expression {
private:
	ExprPtr m_obj;
	ExprPtr m_name;

public:
	GetExpression(ExprPtr obj, ExprPtr name)
		: m_obj(std::move(obj)), m_name(std::move(name)) {
	}
	void evaluate(Evaluator &evaluator) const override {
		evaluator.visit_get_expression(m_obj,
									   m_name);
	}
};

class ModuleExpression : public Expression {
private:
	ExprList m_expressions;

public:
	explicit ModuleExpression(ExprList exprs)
		: m_expressions(std::move(exprs)) {
	}

	void evaluate(Evaluator &evaluator) const override {
		evaluator.visit_module_definition(m_expressions);
	}
};

class ExpressionStatement : public Statement {
private:
    const ExprPtr expr;

public:
    explicit ExpressionStatement(ExprPtr in_expr)
        : expr(std::move(in_expr)) {}

    void execute(Evaluator& evaluator) const override {
        expr->evaluate(evaluator);
    }
};

class IfStatement : public Statement {
private:
    ExprPtr m_cond;
    StatementPtr m_body;
    StatementPtr m_else;

public:
    explicit IfStatement(ExprPtr cond, StatementPtr body, StatementPtr else_branch)
            : m_cond(std::move(cond)),
              m_body(std::move(body)),
              m_else(std::move(else_branch)) {
    }

    void execute(Evaluator &evaluator) const override {
        evaluator.visit_if_statement(m_cond,
                                     m_body,
                                     m_else);
    }
};

class WhileStatement : public Statement {
private:
    ExprPtr m_cond;
    StatementPtr m_body;

public:
    explicit WhileStatement(ExprPtr cond, StatementPtr body)
            : m_cond(std::move(cond)), m_body(std::move(body)) {
    }
    void execute(Evaluator &evaluator) const override {
        evaluator.visit_while_statement(m_cond,
                                        m_body);
    }
};

class ForStatement : public Statement {
private:
    std::string m_name;
    ExprPtr m_iterable;
    StatementPtr m_body;

public:
    explicit ForStatement(std::string name, ExprPtr iterable, StatementPtr body)
            : m_name(std::move(name)),
              m_iterable(std::move(iterable)),
              m_body(std::move(body)) {
    }
    void execute(Evaluator &evaluator) const override {
        evaluator.visit_for_statement(m_name,
                                      m_iterable,
                                      m_body);
    }
};


class FunDefStatement : public Statement {
private:
    String m_name;
    Names m_args;
    const StatementPtr m_body;

public:
    explicit FunDefStatement(String name, Names arg_names, StatementPtr body)
            : m_name(std::move(name)), m_args(std::move(arg_names)),m_body(std::move(body)) {
    }
    void execute(Evaluator &evaluator) const override {
        evaluator.visit_fun_def_statement(m_name,
                                          m_args,
                                          m_body);
    }
};

class BlockStatement : public Statement {
private:
    StatementList m_body;

public:
    explicit BlockStatement(StatementList body)
            : m_body(std::move(body)) {
    }
    void execute(Evaluator &evaluator) const override {
        evaluator.visit_block_statement(m_body);
    }
};

} // namespace CL
