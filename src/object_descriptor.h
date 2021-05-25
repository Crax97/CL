#pragma once 

#include "commons.hpp"
#include "value.hpp"
#include <unordered_map>
#include <utility>

namespace CL {

	struct PropertyDescriptor {

		virtual RuntimeValue get(void* instance_ptr) = 0;
	};

	template<typename InstanceType, typename PropertyType>
	PropertyDescriptor* get_property_descriptor(PropertyType InstanceType::*in_property_ptr) { 
		
		struct TypedDescriptor : public PropertyDescriptor {

			PropertyType InstanceType::*property_ptr;
		
			explicit TypedDescriptor(PropertyType InstanceType::*_property_ptr)
				: property_ptr(_property_ptr) {}

			RuntimeValue get(void* instance_ptr) override {
				InstanceType instance = *reinterpret_cast<InstanceType*>(instance_ptr);
				PropertyType value = instance.*property_ptr;
				return RuntimeValue(value);
			}
		};
		return static_cast<PropertyDescriptor*>(new TypedDescriptor(in_property_ptr));
	}

	class ObjectDescriptor : public std::enable_shared_from_this<ObjectDescriptor>, public Callable {
	private:



	String class_name;
	std::unordered_map<String, std::shared_ptr<PropertyDescriptor>> instance_props;
	//std::unordered_map<String, Method> instance_methods;

	public:
	explicit ObjectDescriptor(String in_class_name)
		: class_name{std::move(in_class_name)} {}

	template<class DescribedClass, typename Type>
	void bind_property(const String& property_name, Type DescribedClass::*property_ptr) {
		auto property_descriptor = get_property_descriptor<DescribedClass, Type>(property_ptr);
		instance_props.insert(std::make_pair(property_name, std::shared_ptr<PropertyDescriptor>(property_descriptor)));
	}

	template<class DescribedClass, typename Type>
	void bind_static_property(const String& property_name, Type DescribedClass::*property_ptr) {
		TODO();
	}
	
	template<class DescribedClass, typename ReturnType, typename... Args>
	void bind_method(const String& method_name, ReturnType (DescribedClass::*method_ptr)(Args...)) {
		TODO();
	}

	template <class ConcreteDescriptor>
	static std::shared_ptr<ObjectDescriptor> get_descriptor() {
		static std::shared_ptr<ObjectDescriptor> descriptor = std::make_shared<ConcreteDescriptor>();
		return descriptor;
	}

	void bind_into_env(RuntimeEnvPtr env);

	bool has_property(const String& prop_name) {
		return instance_props.find(prop_name) != instance_props.end();
	}

	RuntimeValue get_property(const String& prop_name, const std::shared_ptr<class Object>& instance) {
		if(has_property(prop_name)) {
			return instance_props[prop_name]->get(instance.get());
		}
	}

	std::string to_string() const noexcept override {
		return String("Class Descriptor for ") + class_name; 
	}
	std::string string_repr() const noexcept {
		return to_string();
	}
	};
}