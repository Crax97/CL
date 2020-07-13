#pragma once
#include "commons.hpp"

namespace Calculator {
void inject_import_function(Env<RuntimeValue>& env);
void inject_stdlib_functions(Env<RuntimeValue>& env);
void inject_math_functions(Env<RuntimeValue>& env);
}
