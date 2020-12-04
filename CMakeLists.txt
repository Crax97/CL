cmake_minimum_required(VERSION 3.13)
project(calculator)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

set(SOURCES
        src/main.cpp
        src/commons.cpp
        src/string_visitor.cpp
        src/lexer.cpp
        src/parser.cpp
        src/tokens.cpp
        src/value.cpp
        src/ast_evaluator.cpp
        src/environment.cpp
        src/std_lib.cpp
        src/script.cpp src/script.h src/helpers.h src/dictionary.cpp src/dictionary.h src/vm_ast_evaluator.cpp src/vm_ast_evaluator.h)


set(CMAKE_CXX_STANDARD 17)

add_executable(calc ${SOURCES})
