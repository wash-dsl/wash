#include "forces.hpp"

namespace wash {

namespace refactor {

namespace forces {

    StatementMatcher GetForceScalarMatcher = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        on(hasType(cxxRecordDecl(hasName("Particle")))),
        callee(cxxMethodDecl(hasName("get_force_scalar"))),
        hasArgument(0, ignoringImplicit(stringLiteral().bind("forceName")))
    ).bind("callExpr"));

    StatementMatcher GetForceVectorMatcher = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        on(hasType(cxxRecordDecl(hasName("Particle")))),
        callee(cxxMethodDecl(hasName("get_force_vector"))),
        hasArgument(0, ignoringImplicit(stringLiteral().bind("forceName")))
    ).bind("callExpr"));

    template <ForceType type>
    void HandleGetForce(const MatchFinder::MatchResult &Result, Replacements& Replace) {
        const auto *call = Result.Nodes.getNodeAs<CXXMemberCallExpr>("callExpr");
        const auto *forceName = Result.Nodes.getNodeAs<clang::StringLiteral>("forceName");
        const Expr *objectExpr = call->getImplicitObjectArgument();

        if (!call || !forceName) {
            std::cout << "Matched a node with no call or forceName found" << std::endl;
            throw std::runtime_error("Get call match had no force name or call node");
        }

        constexpr const char *kindString = (type == ForceType::SCALAR) ? "scalar" : "vector";

        std::string forceNameStr = forceName->getString().str();
        std::string objectCodeStr = getSourceText(Result.Context, objectExpr->getSourceRange()).value();
        std::string replacementStr = "(wash::" + (std::string)kindString + "_force_" + forceNameStr + ")[" +
                                        objectCodeStr + ".get_id()]";

        auto Err = Replace.add(Replacement(
            *Result.SourceManager, CharSourceRange::getTokenRange(call->getSourceRange()), replacementStr));

        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
        } else {
            std::cout << "\tReplaced a " << kindString << " force get [[" << forceNameStr << "]]" << std::endl;
        }
    }

    WashCallbackFn HandleGetForceScalar = &HandleGetForce<ForceType::SCALAR>;
    WashCallbackFn HandleGetForceVector = &HandleGetForce<ForceType::VECTOR>;
    
    StatementMatcher SetForceScalarMatcher = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        on(hasType(cxxRecordDecl(hasName("Particle")))),
        callee(cxxMethodDecl(hasName("set_force_scalar"))),
        hasArgument(0, ignoringImplicit(stringLiteral().bind("forceName"))),
        hasArgument(1, expr().bind("setValue"))
    ).bind("callExpr"));

    StatementMatcher SetForceVectorMatcher = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        on(hasType(cxxRecordDecl(hasName("Particle")))),
        callee(cxxMethodDecl(hasName("set_force_vector"))),
        hasArgument(0, ignoringImplicit(stringLiteral().bind("forceName"))),
        hasArgument(1, expr().bind("setValue"))
    ).bind("callExpr"));

    template <ForceType type>
    void HandleSetForce(const MatchFinder::MatchResult &Result, Replacements& Replace) {
        const auto *call = Result.Nodes.getNodeAs<CXXMemberCallExpr>("callExpr");
        const auto *forceName = Result.Nodes.getNodeAs<clang::StringLiteral>("forceName");

        const auto setValue = Result.Nodes.getNodeAs<Expr>("setValue");
        const Expr *objectExpr = call->getImplicitObjectArgument();

        if (!call || !forceName) {
            std::cout << "Matched a node with no call or forceName found" << std::endl;
            throw std::runtime_error("Set call match had no force name or call node");
        }

        constexpr const char *kindString = (type == ForceType::SCALAR) ? "scalar" : "vector";

        std::string forceNameStr = forceName->getString().str();
        std::string objectCodeStr = getSourceText(Result.Context, objectExpr->getSourceRange()).value();
        std::string setValueStr = getSourceText(Result.Context, setValue->getSourceRange()).value();

        std::string replacementStr = "(wash::" + (std::string)kindString + "_force_" + forceNameStr + ")[" +
                                        objectCodeStr + ".get_id()] = " + setValueStr;

        auto Err = Replace.add(Replacement(
            *Result.SourceManager, CharSourceRange::getTokenRange(call->getSourceRange()), replacementStr));

        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
        } else {
            std::cout << "\tReplaced a " << kindString << " force set [[" << forceNameStr << "]]"<< std::endl;
        }
    }

    WashCallbackFn HandleSetForceScalar = &HandleSetForce<ForceType::SCALAR>;
    WashCallbackFn HandleSetForceVector = &HandleSetForce<ForceType::VECTOR>;

}

}

}