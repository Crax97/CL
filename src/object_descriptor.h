#pragma once 

#include "commons.hpp"
#include "value.hpp"

namespace CL {
	class ObjectDescriptor : public std::enable_shared_from_this<ObjectDescriptor>, public Callable, public Indexable {
	private:
	String class_name;

	public:
	ObjectDescriptor(String in_class_name)
		: class_name{in_class_name} {}

	template<class DescribedClass, typename Type>
	void bind_property(String property_name, Type DescribedClass::*property_ptr) {
		TODO();
	}

	template<class DescribedClass, typename Type>
	void bind_static_property(String property_name, Type DescribedClass::*property_ptr) {
		TODO();
	}
	
	template<class DescribedClass, typename ReturnType, typename... Args>
	void bind_method(String method_name, ReturnType (DescribedClass::*method_ptr)(Args...)) {
		TODO();
	}

		template <class ConcreteDescriptor>
	static std::shared_ptr<ObjectDescriptor> get_descriptor() {
		static std::shared_ptr<ObjectDescriptor> descriptor = std::make_shared<ConcreteDescriptor>();
		return descriptor;
	}

	void bind_into_env(RuntimeEnvPtr env);

	virtual void set(const RuntimeValue &, RuntimeValue v) override;
	virtual RuntimeValue &get(const RuntimeValue &) override;

	virtual std::string to_string() const noexcept {
		return String("Class Descriptor for ") + class_name; 
	}
	virtual std::string string_repr() noexcept {
		return to_string();
	}
	};
}