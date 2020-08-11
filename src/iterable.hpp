//
// Created by crax on 8/10/20.
//
#pragma once
#include "dictionary.h"
namespace CL {
class Iterable : public Dictionary {
public:
	Iterable() {
		std::function has_next_lambda = [this]() {
			return static_cast<bool>(this->has_next());
		};
		std::function next_lambda = [this]() {
			return static_cast<RuntimeValue>(this->next());
		};
		set_named("__has_next",
				  CL::make_function(has_next_lambda));
		set_named("__next", CL::make_function(next_lambda));
	}
	virtual bool has_next() const = 0;
	virtual RuntimeValue next() = 0;
};

}
