#include "std_lib.hpp"
#include "ast_evaluator.hpp"
#include "commons.hpp"
#include "environment.hpp"
#include "exceptions.hpp"
#include "function_callable.hpp"
#include "value.hpp"
#include "script.h"
#include "iterable.hpp"
#include "helpers.h"

#include <cmath>
#include <stdlib.h>
#include <fstream>
#include <memory>
#include <functional>

namespace CL {

class RangeIterator : public Iterable {
private:
	Number m_current, m_end, m_step;

public:
	RangeIterator(Number begin, Number end, Number step)
		: m_current(begin), m_end(end), m_step(step) {
	}

	[[nodiscard]]
	bool has_next() const override {
		return m_current < m_end;
	}

	[[nodiscard]]
	RuntimeValue next() override {
		auto v = m_current;
		m_current += m_step;
		return RuntimeValue(v);
	}
};

class FileObject : public Iterable {
private:
	std::fstream m_stream;
	std::ios_base::openmode mode;
	bool is_readable() { return mode & std::iostream::in; };
	bool is_writable() { return mode & std::iostream::out; };
	bool is_appendable() { return mode & std::iostream::app; };
	void build_lambdas() {
		std::function write_lam = [this](const std::string &str) {
			this->write(str);
		};
		std::function readline_lam = [this]() {
			return this->readline();
		};
		std::function close_lam = [this]() {
			return this->close();
		};
		std::function flush_lam = [this]() {
			if(!this->m_stream.is_open())
				throw RuntimeException("This file is not open");
			this->m_stream.flush();
		};
		set_named("write", CL::make_function(write_lam));
		set_named("readline", CL::make_function(readline_lam));
		set_named("close", CL::make_function(close_lam));
		set_named("flush", CL::make_function(flush_lam));
	}
public:

	explicit FileObject(const std::string &path, const std::string &modestr) {
		mode = static_cast<std::_Ios_Openmode>(0);
		if(modestr.find('r') != std::string::npos)
			mode |= std::ios_base::in;
		if(modestr.find('w') != std::string::npos)
			mode |= std::ios_base::out;
		if(modestr.find('a') != std::string::npos)
			mode |= std::ios_base::app | std::ios_base::out;

		m_stream.open(path, mode);

		if(!m_stream.is_open())
			throw RuntimeException(
				"Could not open or create file located at: " + path);

		build_lambdas();
	}

	void write(const std::string &line) {
		if(!m_stream.is_open())
			throw RuntimeException("This file is not open");
		if(is_writable()) {
			m_stream << line;
		} else {
			throw RuntimeException("This file is not writable");
		}
	}

	void close() {
		if(!m_stream.is_open())
			throw RuntimeException("This file is already closed");
		m_stream << std::flush;
		m_stream.close();
		if(m_stream.fail()) {
			throw RuntimeException("Failure closing the file");
		}
	}

	std::string readline() {
		if(!m_stream.is_open())
			throw RuntimeException("This file is not open");
		if(is_readable()) {
			std::string line;
			std::getline(m_stream, line);
			return line;
		} else {
			throw RuntimeException("This file is not writable");
		}
	}

	[[nodiscard]]
	bool has_next() const override {
		return !m_stream.eof();
	}

	[[nodiscard]]
	RuntimeValue next() override {
		return readline();
	}

	std::string to_string() const override {
		return "File " + addr_to_hex_str(*this);
	}
	std::string string_repr() const override {
		return "File " + addr_to_hex_str(*this);
	}
};

std::optional<RuntimeValue> import_impl(const std::string &path,
										const RuntimeEnvPtr &env) {
	auto script = Script::from_file(path, env);
	return script.run();
}

std::optional<RuntimeValue> range(Number begin, Number end, Number step) {
	return RuntimeValue(std::make_shared<RangeIterator>(begin, end, step));
}

std::optional<RuntimeValue> open(const std::string &file_path,
								 const std::string &openmode) {
	auto file_it = std::make_shared<FileObject>(file_path, openmode);
	return RuntimeValue(file_it);
}
Number deg2rad(double deg) {
	return deg * M_PI / 180.0;
}
Number rad2deg(double rad) {
	return rad * 180.0 / M_PI;
}

void inject_import_function(const RuntimeEnvPtr &parent_env) {
	static std::function fn = [parent_env](const std::string &str) {
		return import_impl(str,
						   parent_env);
	};
	static auto fun_ptr = CL::make_function(fn);
	parent_env->assign("import", RuntimeValue(fun_ptr));
}

void inject_stdlib_functions(const RuntimeEnvPtr &env) {
	auto exit_impl = CL::make_function(exit);
	auto range_impl = CL::make_function(range);
	auto open_impl = CL::make_function(open);
	static auto input_impl =
		std::make_shared<LambdaStyleFunction>([](const auto &_args) {
												  String line;
												  std::getline(std::cin, line);
												  return RuntimeValue(line);
											  },
											  0);
	static auto
		print_impl = std::make_shared<VoidFunction>([](const Args &args) {
														for (const auto &arg : args) {
															std::cout << arg.to_string() << " ";
														}
														std::cout << "\n";
													},
													VAR_ARGS);

	static auto
		repr_impl = std::make_shared<LambdaStyleFunction>([](const Args &args) {
															  return args[0].string_representation();
														  },
														  1);
	env->assign("exit", exit_impl);
	env->assign("input", RuntimeValue(input_impl));
	env->assign("print", RuntimeValue(print_impl));
	env->assign("repr", RuntimeValue(repr_impl));
	env->assign("range", range_impl);
	env->assign("open", open_impl);
}

void inject_math_functions(const RuntimeEnvPtr &env) {
	auto dict_object = std::make_shared<Dictionary>();

	dict_object->set_named("sin", CL::make_function(sin));
	dict_object->set_named("cos", CL::make_function(cos));
	dict_object->set_named("tan", CL::make_function(tan));
	dict_object->set_named("atan2", CL::make_function(atan2));
	dict_object->set_named("exp", CL::make_function(exp));
	dict_object->set_named("log10", CL::make_function(log10));
	dict_object->set_named("log2", CL::make_function(log2));
	dict_object->set_named("deg2rad", CL::make_function(deg2rad));
	dict_object->set_named("rad2deg", CL::make_function(rad2deg));

	// Explicit instantiation because there are more overloads for abs
	dict_object->set_named("abs", CL::make_function<double, double>(abs));
	dict_object->set_named("PI", M_PI);
	dict_object->set_named("E", M_E);
	env->assign("Math", RuntimeValue(dict_object), true);
}
};
