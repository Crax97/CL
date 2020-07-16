#pragma once
#include "commons.hpp"

namespace Calculator {
void inject_import_function(RuntimeEnv& env);
void inject_stdlib_functions(RuntimeEnv& env);
void inject_math_functions(RuntimeEnv& env);
}
