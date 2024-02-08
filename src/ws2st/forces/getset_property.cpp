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

    WashCallbackFn HandleGetPos = &HandleGetProperty<ForceType::VECTOR, PropertyList::Pos>;
    WashCallbackFn HandleGetVel = &HandleGetProperty<ForceType::VECTOR, PropertyList::Vel>;
    WashCallbackFn HandleGetAcc = &HandleGetProperty<ForceType::VECTOR, PropertyList::Acc>;

    WashCallbackFn HandleGetDensity = &HandleGetProperty<ForceType::SCALAR, PropertyList::Density>;
    WashCallbackFn HandleGetMass = &HandleGetProperty<ForceType::SCALAR, PropertyList::Mass>;
    WashCallbackFn HandleGetSmoothingLength = &HandleGetProperty<ForceType::SCALAR, PropertyList::SmoothingLength>;

    template <ForceType type, PropertyList property>
    void HandleGetProperty(const MatchFinder::MatchResult& Result, Replacements& Replace) {
        const auto *call = Result.Nodes.getNodeAs<CXXMemberCallExpr>("callExpr");
        const Expr *objectExpr = call->getImplicitObjectArgument();

        if (!call) {
            std::cout << "Matched a node with no call found" << std::endl;
            throw std::runtime_error("Matched a get property call with no call node.");
        }

        std::string name = propertyName(property);

        // std::cout << "Picked up " << getSourceText(Result.Context, call->getSourceRange()).value() << std::endl;
        const std::string kindString = (type == ForceType::SCALAR) ? "scalar" : "vector";
        std::string objectCodeStr = getSourceText(Result.Context, objectExpr->getSourceRange()).value();
        std::string replacementStr = "(wash::" + 
            (std::string)kindString + "_force_" + name + ")[" + objectCodeStr + ".get_id()]";

        auto Err = Replace.add(Replacement(
            *Result.SourceManager, CharSourceRange::getTokenRange(call->getSourceRange()), replacementStr));

        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
            throw std::runtime_error("Error handling a match callback.");
        } else {
            std::cout << "\tReplaced a call to get the " << kindString << " property [[" << name << "]]" << std::endl;
        }
    }

    StatementMatcher SetPosMatcher = PropertySetMatcher("set_pos");
    StatementMatcher SetVelMatcher = PropertySetMatcher("set_vel");
    StatementMatcher SetAccMatcher = PropertySetMatcher("set_acc");

    StatementMatcher SetDensityMatcher = PropertySetMatcher("set_density");
    StatementMatcher SetMassMatcher = PropertySetMatcher("set_mass");
    StatementMatcher SetSmoothingLengthMatcher = PropertySetMatcher("set_smoothing_length");

    WashCallbackFn HandleSetPos = &HandleSetProperty<ForceType::VECTOR, PropertyList::Pos>;
    WashCallbackFn HandleSetVel = &HandleSetProperty<ForceType::VECTOR, PropertyList::Vel>;
    WashCallbackFn HandleSetAcc = &HandleSetProperty<ForceType::VECTOR, PropertyList::Acc>;

    WashCallbackFn HandleSetDensity = &HandleSetProperty<ForceType::SCALAR, PropertyList::Density>;
    WashCallbackFn HandleSetMass = &HandleSetProperty<ForceType::SCALAR, PropertyList::Mass>;
    WashCallbackFn HandleSetSmoothingLength = &HandleSetProperty<ForceType::SCALAR, PropertyList::SmoothingLength>;

    template <ForceType type, PropertyList property>
    void HandleSetProperty(const MatchFinder::MatchResult& Result, Replacements& Replace)  {
        const auto *call = Result.Nodes.getNodeAs<CXXMemberCallExpr>("callExpr");
        const auto setValue = Result.Nodes.getNodeAs<Expr>("setValue");
        const Expr *objectExpr = call->getImplicitObjectArgument();

        if (!call) {
            std::cout << "Matched a node with no call found" << std::endl;
            throw std::runtime_error("Matched a set property call with no call node.");
        }

        std::string name = propertyName(property);

        const std::string kindString = (type == ForceType::SCALAR) ? "scalar" : "vector";
        // std::cout << "Picked up " << kindString << " "
        //             << getSourceText(Result.Context, call->getSourceRange()).value() << std::endl;
        std::string objectCodeStr = getSourceText(Result.Context, objectExpr->getSourceRange()).value();
        std::string setValueStr = getSourceText(Result.Context, setValue->getSourceRange()).value();

        std::string replacementStr = "(wash::" + (std::string)kindString + "_force_" + name + ")[" +
                                        objectCodeStr + ".get_id()] = " + setValueStr;

        auto Err = Replace.add(Replacement(
            *Result.SourceManager, CharSourceRange::getTokenRange(call->getSourceRange()), replacementStr));

        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
            throw std::runtime_error("Error handling a match callback.");
        } else {
            std::cout << "\tReplaced a call to set the " << kindString << " property [[" << name << "]]" << std::endl;
        }
    }

    const std::string propertyName(PropertyList property) {
        switch (property) {
            case PropertyList::Pos:
                return "pos";
            case PropertyList::Vel:
                return "vel";
            case PropertyList::Acc:
               return "acc";
            case PropertyList::Density:
                return "density";
            case PropertyList::Mass:
                return "mass";
            case PropertyList::SmoothingLength:
                return "smoothing_length";
            default:
                return "none";
        }
    }

}

}

}