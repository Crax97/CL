#pragma once

#include <string>

#include "nodes.hpp"
#include "stack_based_evaluator.hpp"

namespace CL {
class StringVisitor : public StackMachine<std::string>, public Evaluator {
private:
	uint8_t m_scope{0};
	[[nodiscard]]
	std::string get_tabs() const noexcept;

public:
	std::string get_result() noexcept { return pop(); }

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
	void visit_fun_def_statement(const String& name, const Names &names, const StatementPtr &body) override;
	void visit_block_statement(const StatementList &block) override;
	void visit_break_expression() override;
	void visit_continue_expression() override;
	void visit_return_expression(const ExprPtr &expr) override;
	void visit_if_statement(const ExprPtr &cond,
                            const StatementPtr &expr,
                            const StatementPtr &else_branch) override;
	void visit_while_statement(const ExprPtr &cond,
                               const StatementPtr &body) override;
	void visit_for_statement(const std::string &name,
                             const ExprPtr &iterable,
                             const StatementPtr &body) override;
	void visit_set_expression(const ExprPtr &obj,
							  const ExprPtr &name,
							  const ExprPtr &val) override;
	void visit_get_expression(const ExprPtr &obj, const ExprPtr &name) override;
	void visit_module_definition(const ExprList &list) override;
};
}
