/**
 * @file refactor.hpp
 * @author james
 * @brief Idea is to write out the results of the refactoring using the classes defined here
 * @version 0.1
 * @date 2024-01-22
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once

#include "common.hpp"

namespace wash {

    std::optional<std::string> getSourceText(ASTContext* ctx, SourceRange srcRange);

    namespace refactoring {

        class GetForceScalarRefactor : public tooling::RefactoringCallback {
        public:
            virtual void run(const MatchFinder::MatchResult &Result) {

                const auto *call = Result.Nodes.getNodeAs<CXXMemberCallExpr>("callExpr");
                const auto *forceName = Result.Nodes.getNodeAs<clang::StringLiteral>("forceName");
                const Expr *objectExpr = call->getImplicitObjectArgument();

                if (!call || !forceName) {
                    std::cout << "Matched a node with no call or forceName found" << std::endl;
                    return;
                }

                std::cout << "picked up " << getSourceText(Result.Context, call->getSourceRange()).value() << std::endl;

                std::string forceNameStr = forceName->getString().str();
                std::string objectCodeStr = getSourceText(Result.Context, objectExpr->getSourceRange()).value();
                std::string replacementStr = "force_" + forceNameStr + "[" + objectCodeStr + ".get_id()]";

                auto Err = Replace.add(Replacement(*Result.SourceManager, 
                    CharSourceRange::getTokenRange(call->getSourceRange()),
                    replacementStr 
                ));

                if (Err) {
                    std::cout << llvm::toString(std::move(Err)) << std::endl;
                } else {
                    std::cout << "\tdid a replace\t" << replacementStr <<  std::endl;
                }
            }
        };

        // etc..
    }
    
    extern StatementMatcher getForceScalarMatcher;
    extern StatementMatcher getForceVectorMatcher;

    extern StatementMatcher setForceScalarMatcher;
    extern StatementMatcher setForceVectorMatcher;

    // Add a bunch of matchers to handle calls to the in-built functions as well

    int forceNameRewriting(RefactoringTool& Tool);

    void writeOutReplacements(RefactoringTool& tool);
}