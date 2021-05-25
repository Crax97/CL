#include "object.h"
#include "object_descriptor.h"

namespace CL {

	void Object::set(const RuntimeValue & key, RuntimeValue value) {

	}
	RuntimeValue Object::get(const RuntimeValue & value) {
		return descriptor->get_property(value.as<String>(), shared_from_this());
	}
	void set_named(const std::string &name, RuntimeValue v) {
		TODO();

	}
	RuntimeValue get_named(const std::string &name) {
		TODO();
	}
}