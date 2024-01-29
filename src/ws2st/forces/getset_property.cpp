#include "forces.hpp"

namespace wash {

namespace refactor {

namespace forces {

    const StatementMatcher PropertyGetMatcher(const char* propertyName) {
        return traverse(TK_IgnoreUnlessSpelledInSource, 
            cxxMemberCallExpr(
                on(hasType(cxxRecordDecl(hasName("Particle")))),
                callee(cxxMethodDecl(hasName(propertyName)))
            ).bind("callExpr")
        );
    }

    const StatementMatcher PropertySetMatcher(const char* propertyName) {
        return traverse(TK_IgnoreUnlessSpelledInSource, 
            cxxMemberCallExpr(
                on(hasType(cxxRecordDecl(hasName("Particle")))),
                callee(cxxMethodDecl(hasName(propertyName))),
                hasArgument(0, ignoringImplicit(expr().bind("setValue")))
            ).bind("callExpr")
        );
    }

    StatementMatcher GetPosMatcher = PropertyGetMatcher("get_pos");
    StatementMatcher GetVelMatcher = PropertyGetMatcher("get_vel");
    StatementMatcher GetAccMatcher = PropertyGetMatcher("get_acc");

    StatementMatcher GetDensityMatcher = PropertyGetMatcher("get_density");
    StatementMatcher GetMassMatcher = PropertyGetMatcher("get_mass");
    StatementMatcher GetSmoothingLengthMatcher = PropertyGetMatcher("get_smoothing_length");

    template <ForceType type>
    void HandleGetProperty<type>::run(const MatchFinder::MatchResult &Result) {
        const auto *call = Result.Nodes.getNodeAs<CXXMemberCallExpr>("callExpr");
        const Expr *objectExpr = call->getImplicitObjectArgument();

        if (!call) {
            std::cout << "Matched a node with no call found" << std::endl;
            throw std::runtime_error("Matched a get property call with no call node.");
        }

        std::cout << "picked up " << getSourceText(Result.Context, call->getSourceRange()).value() << std::endl;
        constexpr const char *kindString = (type == ForceType::SCALAR) ? "scalar" : "vector";
        std::string objectCodeStr = getSourceText(Result.Context, objectExpr->getSourceRange()).value();
        std::string replacementStr =
            (std::string)kindString + "_force_" + name + "[" + objectCodeStr + ".get_id()]";

        auto Err = Replace.add(Replacement(
            *Result.SourceManager, CharSourceRange::getTokenRange(call->getSourceRange()), replacementStr));

        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
        } else {
            std::cout << "Replaced a call to get the " << kindString << " property (" << name << ")" << std::endl;
        }
    }

    StatementMatcher SetPosMatcher = PropertySetMatcher("set_pos");
    StatementMatcher SetVelMatcher = PropertySetMatcher("set_vel");
    StatementMatcher SetAccMatcher = PropertySetMatcher("set_acc");

    StatementMatcher SetDensityMatchher = PropertySetMatcher("set_density");
    StatementMatcher SetMassMatcher = PropertySetMatcher("set_mass");
    StatementMatcher SetSmoothingLenngthMatcher = PropertySetMatcher("set_smoothing_length");

    template <ForceType type>
    void HandleSetProperty<type>::run(const MatchFinder::MatchResult &Result) {
        const auto *call = Result.Nodes.getNodeAs<CXXMemberCallExpr>("callExpr");
        const auto setValue = Result.Nodes.getNodeAs<Expr>("setValue");
        const Expr *objectExpr = call->getImplicitObjectArgument();

        if (!call) {
            std::cout << "Matched a node with no call found" << std::endl;
            throw std::runtime_error("Matched a set property call with no call node.");
        }

        constexpr const char *kindString = (type == ForceType::SCALAR) ? "scalar" : "vector";
        std::cout << "picked up " << kindString << " "
                    << getSourceText(Result.Context, call->getSourceRange()).value() << std::endl;
        std::string objectCodeStr = getSourceText(Result.Context, objectExpr->getSourceRange()).value();
        std::string setValueStr = getSourceText(Result.Context, setValue->getSourceRange()).value();

        std::string replacementStr = "(*wash::" + (std::string)kindString + "_force_" + name + ")[" +
                                        objectCodeStr + ".get_id()] = " + setValueStr;

        auto Err = Replace.add(Replacement(
            *Result.SourceManager, CharSourceRange::getTokenRange(call->getSourceRange()), replacementStr));

        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
        } else {
            std::cout << "Replaced a call to set the " << kindString << " property (" << name << ")" << std::endl;
        }
    }

}

}

}