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

namespace ws2st {

    std::shared_ptr<WashProgramMeta> program_meta = std::make_shared<WashProgramMeta>();

namespace refactor {

    RefactoringToolConfiguration refactoring_stages {
        {
            AllFiles,
            // Detect kernels
            WashRefactoringAction(&dependency_detection::AddForceKernelMatcher, &dependency_detection::RegisterForceKernel),
        },
        {
            AllFiles,
            // Detect force dependencies
            WashRefactoringAction(&dependency_detection::ForceAssignmentInFunction, &dependency_detection::RegisterForceAssignment),
            WashRefactoringAction(&dependency_detection::PosAssignmentInFunction, &dependency_detection::RegisterPosAssignment),
            WashRefactoringAction(&dependency_detection::VelAssignmentInFunction, &dependency_detection::RegisterVelAssignment),
            WashRefactoringAction(&dependency_detection::AccAssignmentInFunction, &dependency_detection::RegisterAccAssignment),

            WashRefactoringAction(&dependency_detection::ForceReadInFunction, &dependency_detection::RegisterForceRead),
            WashRefactoringAction(&dependency_detection::PosReadInFunction, &dependency_detection::RegisterPosRead),
            WashRefactoringAction(&dependency_detection::VelReadInFunction, &dependency_detection::RegisterVelRead),
            WashRefactoringAction(&dependency_detection::AccReadInFunction, &dependency_detection::RegisterAccRead),
        },

        // 0th pass: Information gathering about the simulation
        {
            AllFiles,
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
            AllFiles,
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
            AllFiles,
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

    bool RefactoringToolConfiguration::run(const WashOptions& opts) {
        int clang_argc = 0;
        std::vector<const char*> clang_args = {};
        /// TODO: populate the arguments then make them c_str
        // std::transform(argv.cbegin(), argv.cend(), std::back_inserter(clang_args),
        //      [&clang_argc](const std::string& s) { clang_argc++; return s.c_str(); });

        const char** clang_argsv = (const char**)(clang_args.data());
        auto clangOptsParser = CommonOptionsParser::create(clang_argc, clang_argsv, WashS2STCategory);
        if (!clangOptsParser) {
            llvm::errs() << clangOptsParser.takeError();
            throw std::runtime_error("Error: Couldn't create Clang Options Parser");
        }

        int passno = 0;
        for (auto& pass : refactoring_stages) {
            std::cout << "Starting Refactor Pass " << passno << std::endl;

            CommonOptionsParser& passParser = clangOptsParser.get();
            RefactoringTool passTool(passParser.getCompilations(), passParser.getSourcePathList());
            ASTMatchRefactorer matcher (passTool.getReplacements());

            std::vector<WashMatchCallback> matchCallbacks;
            std::vector<std::variant<StatementMatcher*, DeclarationMatcher*>> matchers;
            for (auto action : pass.actions()) {
                matchCallbacks.emplace_back(action.getCallbackFn());
                matchers.emplace_back(action.getMatcher());
            }

            for (size_t i = 0; i < pass.actions().size(); i++) {
                WashMatchCallback* callback = &matchCallbacks.at(i);
                std::visit([&matcher, callback](auto&& astmatch){
                    matcher.addMatcher(*astmatch, callback);
                }, matchers.at(i));
            }

            int success = passTool.runAndSave(newFrontendActionFactory(&matcher).get());

            if (success != 0) {
                throw std::runtime_error("Pass " + std::to_string(passno) + " failed with tool showing non-zero exit code");
            }

            passno++;
        }

        return true;
    }

    /// TODO: Add more implementations and their refactoring tool configurations in here
    void runRefactoring(const WashOptions& opts) {
        if (opts.impl == Implementations::west) {
            refactoring_stages.run(opts);
        }
    }
}

}