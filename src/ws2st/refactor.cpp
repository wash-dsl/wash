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

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory WashS2STCategory("Wash S2S Translator");

namespace wash {

    std::shared_ptr<WashProgramMeta> program_meta;

namespace refactor {

    // template<typename F> std::function<F> make_function(F*);

    std::vector<RefactorPass> refactoring_stages {
        // 1st pass: registration, gets
        {
            WashRefactoringAction(forces::AddForceVectorMatcher, forces::HandleRegisterForcesVector),
            WashRefactoringAction(forces::AddForceScalarMatcher, forces::HandleRegisterForcesScalar),
            WashRefactoringAction(forces::GetForceScalarMatcher, forces::HandleGetForceScalar),
            WashRefactoringAction(forces::GetForceVectorMatcher, forces::HandleGetForceVector),
            
            WashRefactoringAction(forces::GetPosMatcher, forces::HandleGetPos),
            WashRefactoringAction(forces::GetVelMatcher, forces::HandleGetVel),
            WashRefactoringAction(forces::GetAccMatcher, forces::HandleGetAcc),
        
            WashRefactoringAction(forces::GetDensityMatcher, forces::HandleGetDensity),
            WashRefactoringAction(forces::GetMassMatcher, forces::HandleGetMass),
            WashRefactoringAction(forces::GetSmoothingLengthMatcher, forces::HandleGetSmoothingLength),
        },

        // 2nd pass: sets, decl
        {
            WashRefactoringAction(forces::SetForceScalarMatcher, forces::HandleSetForceScalar),
            WashRefactoringAction(forces::SetForceVectorMatcher, forces::HandleSetForceVector),

            WashRefactoringAction(forces::InsertForcesDefinitionMatcher, &forces::HandleInsertForcesDefinition),

            WashRefactoringAction(forces::SetPosMatcher, forces::HandleSetPos),
            WashRefactoringAction(forces::SetVelMatcher, forces::HandleSetVel),
            WashRefactoringAction(forces::SetAccMatcher, forces::HandleSetAcc),
        
            WashRefactoringAction(forces::SetDensityMatcher, forces::HandleSetDensity),
            WashRefactoringAction(forces::SetMassMatcher, forces::HandleSetMass),
            WashRefactoringAction(forces::SetSmoothingLengthMatcher, forces::HandleSetSmoothingLength),

            // WashRefactoringAction(forces::IOForcesLoopMatcher, &forces::HandleIOForcesLoop),
        }

    };

    void runRefactoringStages(std::vector<std::string> argv) {
        int new_argc = 0;
        std::vector<const char*> args_cstr = {};
        std::transform(argv.cbegin(), argv.cend(), std::back_inserter(args_cstr),
                    [&new_argc](const std::string& s) { new_argc++; return s.c_str(); });

        const char** new_argv = (const char**)(args_cstr.data());

        auto ExpectedParser = CommonOptionsParser::create(new_argc, new_argv, WashS2STCategory);

        if (!ExpectedParser) {
            // Fail gracefully for unsupported options.
            llvm::errs() << ExpectedParser.takeError();
            throw std::runtime_error("Couldn't create parser");
        }

        size_t passno = 0;

        for (auto& pass : refactoring_stages) {
            CommonOptionsParser& passParser = ExpectedParser.get();    
            RefactoringTool passTool(passParser.getCompilations(), passParser.getSourcePathList());
            ASTMatchRefactorer matcher(passTool.getReplacements());

            for (auto action : pass) {
                action.apply(matcher);
            }

            int success = passTool.runAndSave(newFrontendActionFactory(&matcher).get());

            if (success != 0) {
                throw std::runtime_error("Pass failed with tool shwoing non-zero exit code");
            }
        }

    }

}

}