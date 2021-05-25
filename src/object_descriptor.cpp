#include "object_descriptor.h"
#include "commons.hpp"
#include "environment.hpp"

namespace CL {
	void ObjectDescriptor::bind_into_env(RuntimeEnvPtr env) {
		env->bind(this->class_name, std::dynamic_pointer_cast<Callable>(shared_from_this()));
	}

	void ObjectDescriptor::set(const RuntimeValue &, RuntimeValue v) {
		TODO()
	}

	RuntimeValue& ObjectDescriptor::get(const RuntimeValue &) {
		TODO()
	}
}