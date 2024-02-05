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

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nThis is the WaSH source-to-source translator tool.\n");

namespace wash {

namespace refactor {

    std::vector<RefactorPass> refactoring_stages {
        // 1st pass: registration, gets
        {
            WashRefactoringAction(forces::AddForceVectorMatcher, &forces::HandleRegisterForces<ForceType::SCALAR>),
            WashRefactoringAction(forces::AddForceScalarMatcher, &forces::HandleRegisterForces<ForceType::VECTOR>),
        },

        // 2nd pass: sets, decl
        {}

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