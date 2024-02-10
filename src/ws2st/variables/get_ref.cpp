#include "variables.hpp"

namespace wash {

namespace refactor {

namespace variables {

    StatementMatcher GetVariableRefMatcher = traverse(TK_IgnoreUnlessSpelledInSource, callExpr(
        hasDescendant(
            declRefExpr(
                to(functionDecl(
                    hasName("wash::get_variable_ref")
                ))
            )
        ),
        hasArgument(0, ignoringImplicit( stringLiteral().bind("variableName") ))
    ).bind("call"));;

    void HandleGetVariableRef(const MatchFinder::MatchResult &Result, Replacements& Replace) {
        const auto call = Result.Nodes.getNodeAs<CallExpr>("call");
        const auto variable_name = Result.Nodes.getNodeAs<clang::StringLiteral>("variableName");

        std::string variable_name_str = variable_name->getString().str();

        std::string replacement_str = "&wash::variable_" + variable_name_str;

        auto Err = Replace.add( Replacement( 
            *Result.SourceManager, CharSourceRange::getTokenRange(call->getSourceRange()), replacement_str ) );
        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
            throw std::runtime_error("Error handling a match callback.");
        } else {
            std::cout << "\tReplaced a variable get ref [[" << variable_name_str << "]]"<< std::endl;
        }
    }

}

}

}