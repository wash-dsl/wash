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

        enum TypeKind { SCALAR, VECTOR };

        template<TypeKind type>
        class GetForceRefactor : public tooling::RefactoringCallback {
        public:
            virtual void run(const MatchFinder::MatchResult &Result) {

                const auto *call = Result.Nodes.getNodeAs<CXXMemberCallExpr>("callExpr");
                const auto *forceName = Result.Nodes.getNodeAs<clang::StringLiteral>("forceName");
                const Expr *objectExpr = call->getImplicitObjectArgument();

                if (!call || !forceName) {
                    std::cout << "Matched a node with no call or forceName found" << std::endl;
                    return;
                }

                constexpr const char* kindString = (type == SCALAR) ? "scalar" : "vector"; 

                std::cout << "picked up " << kindString << " " << getSourceText(Result.Context, call->getSourceRange()).value() << std::endl;

                std::string forceNameStr = forceName->getString().str();
                std::string objectCodeStr = getSourceText(Result.Context, objectExpr->getSourceRange()).value();
                std::string replacementStr = "(*wash::" + (std::string) kindString + "_force_" + forceNameStr + ")[" + objectCodeStr + ".get_id()]";

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

        template<TypeKind type>
        class SetForceRefactor : public tooling::RefactoringCallback {
        public:
            virtual void run(const MatchFinder::MatchResult &Result) {

                const auto *call = Result.Nodes.getNodeAs<CXXMemberCallExpr>("callExpr");
                const auto *forceName = Result.Nodes.getNodeAs<clang::StringLiteral>("forceName");
                
                const auto setValue = Result.Nodes.getNodeAs<Expr>("setValue");
                const Expr *objectExpr = call->getImplicitObjectArgument();

                if (!call || !forceName) {
                    std::cout << "Matched a node with no call or forceName found" << std::endl;
                    return;
                }

                constexpr const char* kindString = (type == SCALAR) ? "scalar" : "vector"; 

                std::cout << "picked up " << kindString << " " << getSourceText(Result.Context, call->getSourceRange()).value() << std::endl;

                std::string forceNameStr = forceName->getString().str();
                std::string objectCodeStr = getSourceText(Result.Context, objectExpr->getSourceRange()).value();
                std::string setValueStr = getSourceText(Result.Context, setValue->getSourceRange()).value();

                std::string replacementStr = 
                    "(*wash::" + (std::string) kindString + "_force_" + forceNameStr + ")[" + objectCodeStr + ".get_id()] = " + setValueStr;

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

        class AddForcDeclarationsRefactor : public tooling::RefactoringCallback {
        private:
            std::unordered_set<std::string> vector_forces;
            std::unordered_set<std::string> scalar_forces;
            
        public:
            AddForcDeclarationsRefactor(const std::unordered_set<std::string>& scalar_f, const std::unordered_set<std::string>& vector_f) : vector_forces(vector_f), scalar_forces(scalar_f) {}
            
            virtual void run(const MatchFinder::MatchResult &Result) {
                const auto decl = Result.Nodes.getNodeAs<CXXRecordDecl>("decl");

                std::cout << "got a " << decl->getNameAsString() << std::endl;

                std::string replacementStr = "";

                for (auto vector_f : vector_forces) {
                    replacementStr += "\n std::vector<SimulationVecT>* vector_force_" + vector_f + ";"; 
                }

                for (auto scalar_f : scalar_forces) {
                    replacementStr += "\n std::vector<double>* scalar_force_" + scalar_f + ";";
                }

                auto Err = Replace.add(Replacement(*Result.SourceManager, 
                    CharSourceRange::getTokenRange(decl->getSourceRange()),
                    replacementStr 
                ));

                if (Err) {
                    std::cout << llvm::toString(std::move(Err)) << std::endl;
                } else {
                    std::cout << "\tdid a replace\t" << replacementStr <<  std::endl;
                }

            }
        };

        template<TypeKind type>
        class GetParticlePropertyRefactor : public tooling::RefactoringCallback {
        private:
            std::string name;
        public:
            GetParticlePropertyRefactor(std::string name) : name(name) {}
            virtual void run(const MatchFinder::MatchResult &Result) {
                
                const auto *call = Result.Nodes.getNodeAs<CXXMemberCallExpr>("callExpr");
                const Expr *objectExpr = call->getImplicitObjectArgument();

                if (!call) {
                    std::cout << "Matched a node with no call found" << std::endl;
                    return;
                }

                std::cout << "picked up " << getSourceText(Result.Context, call->getSourceRange()).value() << std::endl;
                constexpr const char* kindString = (type == SCALAR) ? "scalar" : "vector"; 
                std::string objectCodeStr = getSourceText(Result.Context, objectExpr->getSourceRange()).value();
                std::string replacementStr = "(*wash::" + (std::string) kindString + "_force_" + name + ")[" + objectCodeStr + ".get_id()]";

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

        template<TypeKind type>
        class SetParticlePropertyRefactor : public tooling::RefactoringCallback {
        private:
            std::string name;
        public:
            SetParticlePropertyRefactor(std::string name) : name(name) {}
            virtual void run(const MatchFinder::MatchResult &Result) {
                
                const auto *call = Result.Nodes.getNodeAs<CXXMemberCallExpr>("callExpr");
                const auto setValue = Result.Nodes.getNodeAs<Expr>("setValue");
                const Expr *objectExpr = call->getImplicitObjectArgument();

                if (!call) {
                    std::cout << "Matched a node with no call found" << std::endl;
                    return;
                }

                constexpr const char* kindString = (type == SCALAR) ? "scalar" : "vector"; 
                std::cout << "picked up " << kindString << " " << getSourceText(Result.Context, call->getSourceRange()).value() << std::endl;
                std::string objectCodeStr = getSourceText(Result.Context, objectExpr->getSourceRange()).value();
                std::string setValueStr = getSourceText(Result.Context, setValue->getSourceRange()).value();

                std::string replacementStr = 
                    "(*wash::" + (std::string) kindString + "_force_" + name + ")[" + objectCodeStr + ".get_id()] = " + setValueStr;

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
    }
    
    extern StatementMatcher getForceScalarMatcher;
    extern StatementMatcher getForceVectorMatcher;

    extern StatementMatcher setForceScalarMatcher;
    extern StatementMatcher setForceVectorMatcher;

    extern DeclarationMatcher forceArrays;

    // Adding in a bunch of stuff for the pre-defined properties
    #define PROPERTY_GET_MATCHER(propertyName) traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(on(hasType(cxxRecordDecl(hasName("Particle")))),callee(cxxMethodDecl(hasName(propertyName)))).bind("callExpr"));
    #define PROPERTY_SET_MATCHER(propertyName) traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(on(hasType(cxxRecordDecl(hasName("Particle")))),callee(cxxMethodDecl(hasName(propertyName))),hasArgument(0, ignoringImplicit(expr().bind("setValue")))).bind("callExpr"));
    
    extern StatementMatcher getPosMatcher;
    extern StatementMatcher getVelMatcher;
    extern StatementMatcher getAccMatcher;

    extern StatementMatcher getDensityMatcher;
    extern StatementMatcher getMassMatcher;
    extern StatementMatcher getSLMatcher;

    extern StatementMatcher setPosMatcher;
    extern StatementMatcher setVelMatcher;
    extern StatementMatcher setAccMatcher;

    extern StatementMatcher setDensityMatcher;
    extern StatementMatcher setMassMatcher; 
    extern StatementMatcher setSLMatcher;

    // int forceNameRewriting(RefactoringTool& Tool, const std::unordered_set<std::string>& scalar_f, const std::unordered_set<std::string>& vector_f);
    // void writeOutReplacements(RefactoringTool& tool);

    int getForceRewriting(RefactoringTool& Tool, const std::unordered_set<std::string>& scalar_f, const std::unordered_set<std::string>& vector_f);
    int setForceRewriting(RefactoringTool& Tool);
    int forceDeclRewriting(RefactoringTool& Tool, const std::unordered_set<std::string>& scalar_f, const std::unordered_set<std::string>& vector_f);

}