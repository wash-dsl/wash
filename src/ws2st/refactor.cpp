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
        }

        return std::nullopt;
    }

    int getForceRewriting(RefactoringTool& Tool, const std::unordered_set<std::string>& scalar_f, const std::unordered_set<std::string>& vector_f, uint64_t dimensions) {
        ASTMatchRefactorer finder(Tool.getReplacements());
        refactoring::GetForceRefactor<ForceType::SCALAR> getForcesScalar;
        refactoring::GetForceRefactor<ForceType::VECTOR> getForcesVector;

        refactoring::GetParticlePropertyRefactor<ForceType::SCALAR> getMass("mass");
        refactoring::GetParticlePropertyRefactor<ForceType::SCALAR> getDensity("density");
        refactoring::GetParticlePropertyRefactor<ForceType::SCALAR> getSL("smoothing_length");

        refactoring::GetParticlePropertyRefactor<ForceType::VECTOR> getPos("pos");
        refactoring::GetParticlePropertyRefactor<ForceType::VECTOR> getVel("vel");
        refactoring::GetParticlePropertyRefactor<ForceType::VECTOR> getAcc("acc");

        refactoring::AddForcDeclarationsRefactor forceDeclarations(scalar_f, vector_f);
        refactoring::SimulationVecTRefactor simluationVecT(dimensions);

        finder.addMatcher(simulationVecTMatcher, &simluationVecT);
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
        refactoring::SetForceRefactor<ForceType::SCALAR> setForcesScalar;
        refactoring::SetForceRefactor<ForceType::VECTOR> setForcesVector;

        refactoring::SetParticlePropertyRefactor<ForceType::SCALAR> setMass("mass");
        refactoring::SetParticlePropertyRefactor<ForceType::SCALAR> setDensity("density");
        refactoring::SetParticlePropertyRefactor<ForceType::SCALAR> setSL("smoothing_length");

        refactoring::SetParticlePropertyRefactor<ForceType::VECTOR> setPos("pos");
        refactoring::SetParticlePropertyRefactor<ForceType::VECTOR> setVel("vel");
        refactoring::SetParticlePropertyRefactor<ForceType::VECTOR> setAcc("acc");

        finder.addMatcher(setForceScalarMatcher, &setForcesScalar);
        finder.addMatcher(setForceVectorMatcher, &setForcesVector);

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
        hasArgument(1, expr().bind("setValue"))
    ).bind("callExpr"));

    StatementMatcher setForceVectorMatcher = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        on(hasType(cxxRecordDecl(hasName("Particle")))),
        callee(cxxMethodDecl(hasName("set_force_vector"))),
        hasArgument(0, ignoringImplicit(stringLiteral().bind("forceName"))),
        hasArgument(1, expr().bind("setValue"))
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

    DeclarationMatcher simulationVecTMatcher = traverse(TK_IgnoreUnlessSpelledInSource, typedefNameDecl(
        hasName("SimulationVecT")
    ).bind("typedef"));
}