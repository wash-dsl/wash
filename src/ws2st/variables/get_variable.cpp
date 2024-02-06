/**
 * @file get_variable.cpp
 * @author james
 * @brief Implementation for calls to get variable
 * @version 0.1
 * @date 2024-01-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "variables.hpp"

namespace wash {

namespace refactor {

namespace variables {

    // double get_variable(const std::string& variable);
    StatementMatcher GetVariableMatcher = traverse(TK_IgnoreUnlessSpelledInSource, callExpr(
        hasDescendant(
            declRefExpr(
                to(functionDecl(
                    hasName("wash::get_variable")
                ))
            )
        ),
        hasArgument(0, ignoringImplicit( stringLiteral().bind("variableName") ))
    ).bind("call"));

    void HandleGetVariable(const MatchFinder::MatchResult &Result, Replacements& Replace) {
        const auto call = Result.Nodes.getNodeAs<CallExpr>("call");
        const auto variable_name = Result.Nodes.getNodeAs<clang::StringLiteral>("variableName");

        std::string variable_name_str = variable_name->getString().data();

        std::string replacement_str = "wash::variable_" + variable_name_str;

        auto Err = Replace.add( Replacement( 
            *Result.SourceManager, CharSourceRange::getTokenRange(call->getSourceRange()), replacement_str ) );
        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
        }
    }

}

}

}