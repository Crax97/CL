//
// Created by crax on 7/31/20.
//

#include "script.h"
#include "environment.hpp"
#include "exceptions.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "ast_evaluator.hpp"

#include <fstream>
#include <memory>

namespace CL {
    Script Script::from_file(const std::string &path, RuntimeEnvPtr env) {
        if (env == nullptr) env = std::make_shared<StackedEnvironment>();
        auto file_stream = std::ifstream(path);
        if(!file_stream.is_open()) {
            throw FileNotFoundException(path);
        }

        auto lexer = Lexer(file_stream);
        auto parser = Parser(lexer);

        auto exprs = parser.parse_all();
        return Script(exprs, env);
    }

    Script Script::from_source(const std::string &source, RuntimeEnvPtr env) {
        if (env == nullptr) env = std::make_shared<StackedEnvironment>();
        auto stream = std::stringstream (source);
        auto lexer = Lexer(stream);
        auto parser = Parser(lexer);

        auto exprs = parser.parse_all();
        return Script(exprs, env);
    }

    std::optional<RuntimeValue> Script::run() {
        auto evaluator = ASTEvaluator(m_execution_env);
        for (const auto& expr : m_script_expressions) {
            expr->evaluate(evaluator);
        }
        return evaluator.get_result();
    }
}