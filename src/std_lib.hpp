#pragma once
#include "commons.hpp"

namespace Calculator {
void inject_import_function(const RuntimeEnvPtr& env);
void inject_stdlib_functions(const RuntimeEnvPtr& env);
void inject_math_functions(const RuntimeEnvPtr& env);
}
