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

    std::shared_ptr<WashProgramMeta> program_meta = std::make_shared<WashProgramMeta>();

namespace refactor {

    std::vector<RefactorPass> refactoring_stages {
        {
            // Detect kernels
            WashRefactoringAction(&dependency_detection::AddForceKernelMatcher, &dependency_detection::RegisterForceKernel),},
        
        {
            // Detect force dependencies
            WashRefactoringAction(&dependency_detection::ForceAssignmentInFunction, &dependency_detection::RegisterForceAssignment),
        },

        // 0th pass: Information gathering about the simulation
        {
            // Register Scalar/Vector forces with the simulation
            WashRefactoringAction(&forces::AddForceVectorMatcher, forces::HandleRegisterForcesVector),
            WashRefactoringAction(&forces::AddForceScalarMatcher, forces::HandleRegisterForcesScalar),
            
            // Register Variables with the simulation
            WashRefactoringAction(&variables::RegisterVariableMatcher, &variables::HandleRegisterVariable),
            WashRefactoringAction(&variables::RegisterVariableNoInitMatcher, &variables::HandleRegisterVariable),
            WashRefactoringAction(&meta::SetDimensionMatcher, &meta::HandleSetDimension),

            
        },
        // 1st pass: registration, gets
        {   
            // Rewrite IO functions which inspect the list of forces/force names
            WashRefactoringAction(&meta::DefineForceAccessFnMatcher, &meta::DefineForceAccessFns),
            // Calls to get a force
            WashRefactoringAction(&forces::GetForceScalarMatcher, forces::HandleGetForceScalar),
            WashRefactoringAction(&forces::GetForceVectorMatcher, forces::HandleGetForceVector),
            // Calls to get pre-defined particle properties
            WashRefactoringAction(&forces::GetPosMatcher, forces::HandleGetPos),
            WashRefactoringAction(&forces::GetVelMatcher, forces::HandleGetVel),
            WashRefactoringAction(&forces::GetAccMatcher, forces::HandleGetAcc),
        
            WashRefactoringAction(&forces::GetDensityMatcher, forces::HandleGetDensity),
            WashRefactoringAction(&forces::GetMassMatcher, forces::HandleGetMass),
            WashRefactoringAction(&forces::GetSmoothingLengthMatcher, forces::HandleGetSmoothingLength),

            // Calls to get a variable
            WashRefactoringAction(&variables::GetVariableMatcher, &variables::HandleGetVariable),
            WashRefactoringAction(&variables::GetVariableRefMatcher, &variables::HandleGetVariableRef),
            // Replacement to add the force definitions
            WashRefactoringAction(&forces::InsertForcesDefinitionMatcher, &forces::HandleInsertForcesDefinition),
            WashRefactoringAction(&variables::InsertVariablesDeclarationMatcher, &variables::HandleInsertVariablesDeclaration),

            WashRefactoringAction(&meta::SimulationVecTMatcher, &meta::HandleSimulationVecTMatcher)
        },

        // 2nd pass: set calls
        {
            // Calls to set a force
            WashRefactoringAction(&forces::SetForceScalarMatcher, forces::HandleSetForceScalar),
            WashRefactoringAction(&forces::SetForceVectorMatcher, forces::HandleSetForceVector),
            // Calls to set pre-defined particle properties
            WashRefactoringAction(&forces::SetPosMatcher, forces::HandleSetPos),
            WashRefactoringAction(&forces::SetVelMatcher, forces::HandleSetVel),
            WashRefactoringAction(&forces::SetAccMatcher, forces::HandleSetAcc),
        
            WashRefactoringAction(&forces::SetDensityMatcher, forces::HandleSetDensity),
            WashRefactoringAction(&forces::SetMassMatcher, forces::HandleSetMass),
            WashRefactoringAction(&forces::SetSmoothingLengthMatcher, forces::HandleSetSmoothingLength),

            // Calls to set a variable
            WashRefactoringAction(&variables::SetVariableMatcher, &variables::HandleSetVariable),
        }

    };

    void runRefactoringStages(std::vector<std::string> argv) {
        int new_argc = 0;
        std::vector<const char*> args_cstr = {};
        std::transform(argv.cbegin(), argv.cend(), std::back_inserter(args_cstr),
                    [&new_argc](const std::string& s) { new_argc++; return s.c_str(); });

        const char** new_argv = (const char**)(args_cstr.data());

        for (int i = 0; i < new_argc; i++) {
            std::cout << new_argv[i] << std::endl;
        }

        auto ExpectedParser = CommonOptionsParser::create(new_argc, new_argv, WashS2STCategory);

        if (!ExpectedParser) {
            // Fail gracefully for unsupported options.
            llvm::errs() << ExpectedParser.takeError();
            throw std::runtime_error("Couldn't create parser");
        }

        size_t passno = 0;

        for (auto& pass : refactoring_stages) {
            std::cout << "Starting Refactor Pass " << passno << std::endl;
            CommonOptionsParser& passParser = ExpectedParser.get();    
            RefactoringTool passTool(passParser.getCompilations(), passParser.getSourcePathList());
            ASTMatchRefactorer matcher(passTool.getReplacements());

            std::vector<WashMatchCallback> callbacks;
            for (auto action : pass) {
                callbacks.emplace_back(action.getCallbackFn());
            }

            std::vector<std::variant<StatementMatcher*, DeclarationMatcher*>> matchers;
            for (auto action : pass) {
                // std::cout << "found matcher " << &action.getMatcher() << std::endl;
                matchers.emplace_back(action.getMatcher());
            }

            for (size_t i = 0; i < pass.size(); i++) {
                WashMatchCallback* callback = &callbacks.at(i);
                std::visit([&matcher, callback](auto&& astmatch){
                    // std::cout << "2: creating matcher " << &astmatch << " callback:" << callback << " to:" << reinterpret_cast<const void*>(callback->getCallback()) << std::endl;
                    matcher.addMatcher(*astmatch, callback);
                }, matchers.at(i));
            }

            int success = passTool.runAndSave(newFrontendActionFactory(&matcher).get());

            if (success != 0) {
                throw std::runtime_error("Pass failed with tool showing non-zero exit code");
            }

            passno++;
        }

    }

}

}