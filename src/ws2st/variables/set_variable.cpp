/**
 * @file set_variable.cpp
 * @author james
 * @brief Implementation for calls to set variable
 * @version 0.1
 * @date 2024-01-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "variables.hpp"

namespace ws2st {

namespace refactor {

namespace variables {

    // void set_variable(const std::string& variable, const double value);
    StatementMatcher SetVariableMatcher = traverse(TK_IgnoreUnlessSpelledInSource, callExpr(
        hasDescendant(
            declRefExpr(
                to(functionDecl(
                    hasName("wash::set_variable")
                ))
            )
        ),
        hasArgument(0, ignoringImplicit( stringLiteral().bind("variableName") )),
        hasArgument(1, ignoringImplicit( expr().bind("value") ))
    ).bind("call"));

    void HandleSetVariable(const MatchFinder::MatchResult &Result, Replacements& Replace) {
        const auto call = Result.Nodes.getNodeAs<CallExpr>("call");
        const auto variable_name = Result.Nodes.getNodeAs<clang::StringLiteral>("variableName");
        const auto expr = Result.Nodes.getNodeAs<Expr>("value");

        std::string variable_name_str = variable_name->getString().str();
        std::string expr_source_str = getSourceText(Result.Context, expr->getSourceRange()).value();
        
        std::string replacement_str = "wash::variable_" + variable_name_str + " = " + expr_source_str + ";";

        auto Err = Replace.add( Replacement( 
            *Result.SourceManager, CharSourceRange::getTokenRange(call->getSourceRange()), replacement_str ) );
        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
            throw std::runtime_error("Error handling a match callback.");
        } else {
            std::cout << "\tReplaced a variable set [[" << variable_name_str << "]]"<< std::endl;
        }
    }

}

}

}