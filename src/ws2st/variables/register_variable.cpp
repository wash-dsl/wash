/**
 * @file register_variable.cpp
 * @author james
 * @brief Implementation for calls to register variables
 * @version 0.1
 * @date 2024-01-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "variables.hpp"
#include "../common.hpp"

namespace wash {

namespace refactor {

namespace variables {

    std::vector<std::pair<std::string, std::string>> variable_list = {};

    // void add_variable(const std::string variable, double init_value = 0.0);
    StatementMatcher RegisterVariableMatcher = traverse(TK_IgnoreUnlessSpelledInSource, callExpr(
        hasDescendant(
            declRefExpr(
                to(functionDecl(
                    hasName("wash::add_variable")
                ))
            )
        ),
        hasArgument(0, ignoringImplicit( stringLiteral().bind("variableName") )),
        hasArgument(1, ignoringImplicit( floatLiteral().bind("initValue") ))
    ).bind("call"));

    void HandleRegisterVariable::run(const MatchFinder::MatchResult &Result) {
        const auto call = Result.Nodes.getNodeAs<CallExpr>("call");
        const auto variable_name = Result.Nodes.getNodeAs<clang::StringLiteral>("variableName");
        const auto init_value = Result.Nodes.getNodeAs<clang::FloatingLiteral>("initValue");

        std::string variable_name_str = variable_name->getString().data();
        std::string init_value_str = getSourceText(Result.Context, init_value->getSourceRange()).value();

        variable_list.push_back({ variable_name_str, init_value_str });

        auto Err = Replace.add(Replacement( *Result.SourceManager, CharSourceRange::getTokenRange(call->getSourceRange()), "" ));
        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
        } else {
            std::cout << "Registered Variable " << variable_name_str << ", " << init_value_str << std::endl;
        }
    }

    std::string getVariableDeclarationSource() {
        std::string outputStr;

        for (auto variable : variable_list) {
            outputStr += "double wash::variable_" + variable.first + " = " + variable.second + ";\n";
        }

        return outputStr;
    }

}

}

}