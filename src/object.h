#pragma once

#include "value.hpp"

#include <memory>
#include <sstream>
#include <utility>

namespace CL {
	class Object : public std::enable_shared_from_this<Object>, public Indexable {
		protected:
			std::shared_ptr<class ObjectDescriptor> descriptor = nullptr;

		public:

			explicit Object(std::shared_ptr<class ObjectDescriptor> in_descriptor)
				: descriptor{std::move(in_descriptor)} {}
			// Indexable
			void set(const RuntimeValue &, RuntimeValue v) override;
			RuntimeValue get(const RuntimeValue &) override;
			void set_named(const std::string &name, RuntimeValue v);
			RuntimeValue get_named(const std::string &name);

			std::string to_string() const noexcept override {
				std::stringstream output;
				output << "Object @0x";
				output << std::hex << *reinterpret_cast<const long*>(this) << std::dec;
				output << "\n";
				return output.str();
			};
			std::string string_repr() const noexcept override {
				return to_string();
			}
	};
}