//
// Created by crax on 7/31/20.
//
#pragma once
#include "commons.hpp"

#include <sstream>

namespace Calculator {
class Script {
private:
    RuntimeEnvPtr m_execution_env;
    ExprList  m_script_expressions;
private:
    explicit Script(ExprList list, RuntimeEnvPtr env) :
        m_script_expressions(std::move(list)),
        m_execution_env(std::move(env)) {}
public:
    static Script from_file(const std::string& path, RuntimeEnvPtr env = nullptr);
    static Script from_source(const std::string& source, RuntimeEnvPtr env = nullptr);

    std::optional<RuntimeValue> run();
};
}

