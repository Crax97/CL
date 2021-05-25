#pragma once

#include "commons.hpp"
#include "value.hpp"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace CL {
template<class T>
class Env {
public:
	[[maybe_unused]] virtual void assign(const std::string &,
										 T,
										 bool is_const = false) = 0;

	[[maybe_unused]] virtual T &get(const std::string &) = 0;

	[[maybe_unused]] virtual bool is_bound(const std::string &) = 0;

	[[maybe_unused]] virtual void bind(const std::string &,
									   T,
									   bool is_const = false) = 0;
	[[nodiscard]]
	virtual std::string to_string() const noexcept = 0;
};

class StackedEnvironment
	: public Env<RuntimeValue>,
	  public std::enable_shared_from_this<StackedEnvironment> {
private:
	using Scope = std::unordered_map<std::string, RuntimeValue>;
	Scope m_scope;
	std::unordered_set<std::string> m_consts;
	RuntimeEnvPtr m_parent{nullptr};

public:
	explicit StackedEnvironment(RuntimeEnvPtr parent = nullptr)
		: m_parent(std::move(parent)) {
	}
	void assign(const std::string &,
				RuntimeValue,
				bool is_const = false) override;
	RuntimeValue &get(const std::string &) override;
	bool is_bound(const std::string &) override;
	void bind(const std::string &,
			  RuntimeValue,
			  bool is_const = false) override;
	std::string to_string() const noexcept override;
};
}
