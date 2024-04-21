#include "forces.hpp"

namespace ws2st {

namespace refactor {

namespace forces {

    StatementMatcher GetForceScalarMatcher = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        on(hasType(hasCanonicalType(hasDeclaration(cxxRecordDecl(isSameOrDerivedFrom(hasName("Particle"))))))),
        callee(cxxMethodDecl(hasName("get_force_scalar"))),
        hasArgument(0, ignoringImplicit(stringLiteral().bind("forceName")))
    ).bind("callExpr"));

    StatementMatcher GetForceVectorMatcher = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        on(hasType(hasCanonicalType(hasDeclaration(cxxRecordDecl(isSameOrDerivedFrom(hasName("Particle"))))))),
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
        std::string replacementStr = "wash::" + (std::string)kindString + "_force_" + forceNameStr + "[" +
                                        objectCodeStr + "]";

        auto Err = Replace.add(Replacement(
            *Result.SourceManager, CharSourceRange::getTokenRange(call->getSourceRange()), replacementStr));

        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
            throw std::runtime_error("Error handling a match callback.");
        } else {
            std::cout << "\tReplaced a " << kindString << " force get [[" << forceNameStr << "]]" << std::endl;
        }
    }

    void HandleGetForceVectorCornerstone(const MatchFinder::MatchResult &Result, Replacements& Replace) {
        const auto *call = Result.Nodes.getNodeAs<CXXMemberCallExpr>("callExpr");
        const auto *forceName = Result.Nodes.getNodeAs<clang::StringLiteral>("forceName");
        const Expr *objectExpr = call->getImplicitObjectArgument();

        if (!call || !forceName) {
            std::cout << "Matched a node with no call or forceName found" << std::endl;
            throw std::runtime_error("Get call match had no force name or call node");
        }

        std::string forceNameStr = forceName->getString().str();
        std::string objectCodeStr = getSourceText(Result.Context, objectExpr->getSourceRange()).value();
        std::string replacementStr = "((wash::SimulationVecT) {";

        for (auto dim = 0; dim < program_meta->simulation_dimension; dim++) {
            if (dim > 0) {
                replacementStr += ", ";
            }
            replacementStr += "wash::vector_force_" + forceNameStr + "_" + std::to_string(dim) + "[" + objectCodeStr + "]";
        }
        replacementStr += "})";

        auto Err = Replace.add(Replacement(
            *Result.SourceManager, CharSourceRange::getTokenRange(call->getSourceRange()), replacementStr));

        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
            throw std::runtime_error("Error handling a match callback.");
        } else {
            std::cout << "\tReplaced a vector force get [[" << forceNameStr << "]] (with cornerstone)" << std::endl;
        }
    }

    WashCallbackFn HandleGetForceScalar = &HandleGetForce<ForceType::SCALAR>;
    WashCallbackFn HandleGetForceVector = &HandleGetForce<ForceType::VECTOR>;
    WashCallbackFn HandleGetForceVectorWithCornerstone = &HandleGetForceVectorCornerstone;
    
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

        const std::string kindString = (type == ForceType::SCALAR) ? "scalar" : "vector";

        std::string forceNameStr = forceName->getString().str();
        std::string objectCodeStr = getSourceText(Result.Context, objectExpr->getSourceRange()).value();
        std::string setValueStr = getSourceText(Result.Context, setValue->getSourceRange()).value();

        std::string replacementStr = "wash::" + (std::string)kindString + "_force_" + forceNameStr + "[" +
                                        objectCodeStr + "] = " + setValueStr;

        auto Err = Replace.add(Replacement(
            *Result.SourceManager, CharSourceRange::getTokenRange(call->getSourceRange()), replacementStr));

        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
            throw std::runtime_error("Error handling a match callback.");
        } else {
            std::cout << "\tReplaced a " << kindString << " force set [[" << forceNameStr << "]]"<< std::endl;
        }
    }

    void HandleSetForceVectorCornerstone(const MatchFinder::MatchResult &Result, Replacements& Replace) {
        const auto *call = Result.Nodes.getNodeAs<CXXMemberCallExpr>("callExpr");
        const auto *forceName = Result.Nodes.getNodeAs<clang::StringLiteral>("forceName");

        const auto setValue = Result.Nodes.getNodeAs<Expr>("setValue");
        const Expr *objectExpr = call->getImplicitObjectArgument();

        if (!call || !forceName) {
            std::cout << "Matched a node with no call or forceName found" << std::endl;
            throw std::runtime_error("Set call match had no force name or call node");
        }

        std::string forceNameStr = forceName->getString().str();
        std::string objectCodeStr = getSourceText(Result.Context, objectExpr->getSourceRange()).value();
        std::string setValueStr = getSourceText(Result.Context, setValue->getSourceRange()).value();

        std::string replacementStr = "{\nwash::SimulationVecT temp = " + setValueStr + ";\n";
        for (auto dim = 0; dim < program_meta->simulation_dimension; dim++) {
            replacementStr += "\twash::vector_force_" + forceNameStr + "_" + std::to_string(dim) + "[" + objectCodeStr + "] = temp[" + std::to_string(dim) + "];\n";
        }
        replacementStr += "}";

        auto Err = Replace.add(Replacement(
            *Result.SourceManager, CharSourceRange::getTokenRange(call->getSourceRange()), replacementStr));

        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
            throw std::runtime_error("Error handling a match callback.");
        } else {
            std::cout << "\tReplaced a vector force set [[" << forceNameStr << "]] (with cornerstone)"<< std::endl;
        }
    }

    WashCallbackFn HandleSetForceScalar = &HandleSetForce<ForceType::SCALAR>;
    WashCallbackFn HandleSetForceVector = &HandleSetForce<ForceType::VECTOR>;
    WashCallbackFn HandleSetForceVectorWithCornerstone = &HandleSetForceVectorCornerstone;

}

}

}