#pragma once

#include "value.hpp"

#include <memory>
#include <sstream>

namespace CL {
	class Object : public Indexable {
		protected:
			std::shared_ptr<class ObjectDescriptor> descriptor = nullptr;

		public:

			Object(std::shared_ptr<class ObjectDescriptor> in_descriptor) 
				: descriptor{in_descriptor} {}
			// Indexable
			virtual void set(const RuntimeValue &, RuntimeValue v) override;
			virtual RuntimeValue &get(const RuntimeValue &) override;
			void set_named(const std::string &name, RuntimeValue v);
			RuntimeValue &get_named(const std::string &name);

			virtual std::string to_string() const noexcept override {
				std::stringstream output;
				output << "Object @0x";
				output << std::hex << *reinterpret_cast<const long*>(this) << std::dec;
				output << "\n";
				return output.str();
			};
			virtual std::string string_repr() const noexcept override {
				return to_string();
			}
	};
}