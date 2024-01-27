/**
 * @file refactor.cpp
 * @author james
 * @brief 
 * @version 0.1
 * @date 2024-01-22
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "refactor.hpp"

namespace wash {

    std::optional<std::string> getSourceText(ASTContext* ctx, SourceRange srcRange) {
        const auto &SM = ctx->getSourceManager();
        const auto langOpts = ctx->getLangOpts();
        if (srcRange.isValid()) {
            auto code = Lexer::getSourceText(CharSourceRange::getTokenRange(srcRange), SM, langOpts);
            return code.str();
        } else {
            return std::nullopt;
        }
    }

    int getForceRewriting(RefactoringTool& Tool, const std::unordered_set<std::string>& scalar_f, const std::unordered_set<std::string>& vector_f) {
        ASTMatchRefactorer finder(Tool.getReplacements());
        refactoring::GetForceRefactor<refactoring::TypeKind::SCALAR> getForcesScalar;
        refactoring::GetForceRefactor<refactoring::TypeKind::VECTOR> getForcesVector;

        refactoring::GetParticlePropertyRefactor<refactoring::SCALAR> getMass("mass");
        refactoring::GetParticlePropertyRefactor<refactoring::SCALAR> getDensity("density");
        refactoring::GetParticlePropertyRefactor<refactoring::SCALAR> getSL("smoothing_length");

        refactoring::GetParticlePropertyRefactor<refactoring::VECTOR> getPos("pos");
        refactoring::GetParticlePropertyRefactor<refactoring::VECTOR> getVel("vel");
        refactoring::GetParticlePropertyRefactor<refactoring::VECTOR> getAcc("acc");

        refactoring::AddForcDeclarationsRefactor forceDeclarations(scalar_f, vector_f);

        finder.addMatcher(forceArrays, &forceDeclarations);

        finder.addMatcher(getForceScalarMatcher, &getForcesScalar);
        finder.addMatcher(getForceVectorMatcher, &getForcesVector);

        finder.addMatcher(getPosMatcher, &getPos);
        finder.addMatcher(getVelMatcher, &getVel);
        finder.addMatcher(getAccMatcher, &getAcc);

        finder.addMatcher(getMassMatcher, &getMass);
        finder.addMatcher(getDensityMatcher, &getDensity);
        finder.addMatcher(getSLMatcher, &getSL);

        int code = Tool.runAndSave(newFrontendActionFactory(&finder).get());

        std::cout << "get call rewriting" << std::endl;
        for (auto repl : Tool.getReplacements()) {
            std::cout << repl.first << std::endl;
            for (auto rrepl : repl.second) {
                std::cout << "\t" << rrepl.toString() << std::endl;
            }
        }

        return code;
    }

    int setForceRewriting(RefactoringTool& Tool) {
        ASTMatchRefactorer finder(Tool.getReplacements());
        refactoring::SetForceRefactor<refactoring::TypeKind::SCALAR> setForcesScalar;
        refactoring::SetForceRefactor<refactoring::TypeKind::VECTOR> setForcesVector;

        refactoring::SetParticlePropertyRefactor<refactoring::SCALAR> setMass("mass");
        refactoring::SetParticlePropertyRefactor<refactoring::SCALAR> setDensity("density");
        refactoring::SetParticlePropertyRefactor<refactoring::SCALAR> setSL("smoothing_length");

        refactoring::SetParticlePropertyRefactor<refactoring::VECTOR> setPos("pos");
        refactoring::SetParticlePropertyRefactor<refactoring::VECTOR> setVel("vel");
        refactoring::SetParticlePropertyRefactor<refactoring::VECTOR> setAcc("acc");

        finder.addMatcher(getForceScalarMatcher, &setForcesScalar);
        finder.addMatcher(getForceVectorMatcher, &setForcesVector);

        finder.addMatcher(setPosMatcher, &setPos);
        finder.addMatcher(setVelMatcher, &setVel);
        finder.addMatcher(setAccMatcher, &setAcc);

        finder.addMatcher(setMassMatcher, &setMass);
        finder.addMatcher(setDensityMatcher, &setDensity);
        finder.addMatcher(setSLMatcher, &setSL);

        int code = Tool.runAndSave(newFrontendActionFactory(&finder).get());

        std::cout << "set call rewriting" << std::endl;
        for (auto repl : Tool.getReplacements()) {
            std::cout << repl.first << std::endl;
            for (auto rrepl : repl.second) {
                std::cout << "\t" << rrepl.toString() << std::endl;
            }
        }

        return code;
    }

    int forceDeclRewriting(RefactoringTool& Tool, const std::unordered_set<std::string>& scalar_f, const std::unordered_set<std::string>& vector_f) {
        ASTMatchRefactorer finder(Tool.getReplacements());
        refactoring::AddForcDeclarationsRefactor forceDeclarations(scalar_f, vector_f);

        finder.addMatcher(forceArrays, &forceDeclarations);

        int code = Tool.runAndSave(newFrontendActionFactory(&finder).get());

        std::cout << "force decl rewriting" << std::endl;
        for (auto repl : Tool.getReplacements()) {
            std::cout << repl.first << std::endl;
            for (auto rrepl : repl.second) {
                std::cout << "\t" << rrepl.toString() << std::endl;
            }
        }

        return code;
    }

    StatementMatcher getForceScalarMatcher = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        on(hasType(cxxRecordDecl(hasName("Particle")))),
        callee(cxxMethodDecl(hasName("get_force_scalar"))),
        hasArgument(0, ignoringImplicit(stringLiteral().bind("forceName")))
    ).bind("callExpr"));

    StatementMatcher getForceVectorMatcher = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        on(hasType(cxxRecordDecl(hasName("Particle")))),
        callee(cxxMethodDecl(hasName("get_force_vector"))),
        hasArgument(0, ignoringImplicit(stringLiteral().bind("forceName")))
    ).bind("callExpr"));

    StatementMatcher setForceScalarMatcher = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        on(hasType(cxxRecordDecl(hasName("Particle")))),
        callee(cxxMethodDecl(hasName("set_force_scalar"))),
        hasArgument(0, ignoringImplicit(stringLiteral().bind("forceName"))),
        hasArgument(1, ignoringImplicit(expr().bind("setValue")))
    ).bind("callExpr"));

    StatementMatcher setForceVectorMatcher = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        on(hasType(cxxRecordDecl(hasName("Particle")))),
        callee(cxxMethodDecl(hasName("set_force_vector"))),
        hasArgument(0, ignoringImplicit(stringLiteral().bind("forceName"))),
        hasArgument(1, ignoringImplicit(expr().bind("setValue")))
    ).bind("callExpr"));

    DeclarationMatcher forceArrays = traverse(TK_IgnoreUnlessSpelledInSource, 
        cxxRecordDecl(hasName("_force_vectors")).bind("decl")
    );

    StatementMatcher getPosMatcher = PROPERTY_GET_MATCHER("get_pos");
    StatementMatcher getVelMatcher = PROPERTY_GET_MATCHER("get_vel");
    StatementMatcher getAccMatcher = PROPERTY_GET_MATCHER("get_acc");
    
    StatementMatcher getDensityMatcher = PROPERTY_GET_MATCHER("get_density");
    StatementMatcher getMassMatcher = PROPERTY_GET_MATCHER("get_mass");
    StatementMatcher getSLMatcher = PROPERTY_GET_MATCHER("get_smoothing_length");

    StatementMatcher setPosMatcher = PROPERTY_SET_MATCHER("set_pos");
    StatementMatcher setVelMatcher = PROPERTY_SET_MATCHER("set_vel");
    StatementMatcher setAccMatcher = PROPERTY_SET_MATCHER("set_acc");
    
    StatementMatcher setDensityMatcher = PROPERTY_SET_MATCHER("set_density");
    StatementMatcher setMassMatcher = PROPERTY_SET_MATCHER("set_mass");
    StatementMatcher setSLMatcher = PROPERTY_SET_MATCHER("set_smoothing_length");
}