#pragma once
#include "commons.hpp"

namespace Calculator {
void inject_import_function(RuntimeEnvPtr env);
void inject_stdlib_functions(RuntimeEnvPtr env);
void inject_math_functions(RuntimeEnvPtr env);
}
