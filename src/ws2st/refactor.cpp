/**
 * @file refactor.cpp
 * @author james
 * @brief
 * @version 0.1
 * @date 2024-01-22
 * 
 * Compile with -DCLANG_LOC="./path/to/clang/version/include" 
 * to change the location of where clang's std header files 
 * are included from. 
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "refactor.hpp"

#ifndef CLANG_LOC
#define CLANG_LOC "/usr/lib64/clang/16/include"
#endif

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory WashS2STCategory("Wash S2S Translator");

namespace ws2st {

    std::shared_ptr<WashProgramMeta> program_meta = std::make_shared<WashProgramMeta>();

    namespace refactor {

        std::vector<std::string> prepareArguments(const WashOptions& opts, const std::vector<std::string>& files) {
            std::vector<std::string> compilation_args = {};

            compilation_args.push_back("hi");

            for (auto& file : files) {
                // std::cout << file << std::endl;
                std::string new_str = file;
                new_str.erase(new_str.find_last_not_of(" \n\r\t") + 1);
                if (new_str == "")
                    continue;  // not sure why we're getting empty file paths here :|
                compilation_args.push_back(new_str);
            }

            compilation_args.push_back("--");

            for (auto& arg : opts.args) {
                compilation_args.push_back(arg);
            }

            compilation_args.push_back("-I" CLANG_LOC);
            compilation_args.push_back("-std=c++17");

            if (args::areMPIFlagsRequired(opts.impl, opts)) {
                for (auto& arg : args::getMPICompileFlags()) {
                    std::string new_str = arg;
                    new_str.erase(new_str.find_last_not_of(" \n\r\t") + 1);
                    compilation_args.push_back(new_str);
                }

                compilation_args.push_back("-Isrc/cornerstone-octree/include");
            }

            if (opts.hdf5) {
                for (auto& arg : args::getHDF5CompileFlags()) {
                    compilation_args.push_back(arg);
                }
            }

            for (auto& arg : args::getImplCompileFlag(opts.impl)) {
                compilation_args.push_back(arg);
            }

            if (program_meta->simulation_dimension == 0) {
                compilation_args.push_back("-DDIM=" + std::to_string(opts.dim));
            } else {
                compilation_args.push_back("-DDIM=" + std::to_string(program_meta->simulation_dimension));
            }

            return compilation_args;
        }

        bool RefactoringToolConfiguration::run(const WashOptions& opts) const {
            int passno = 0;
            for (auto& pass : refactoring_stages) {
                std::cout << "Starting Refactor Pass " << passno << std::endl;
                auto files = pass.files();

                if (files->size() > 0) {
                    auto compilation_args = prepareArguments(opts, *files);
                    int clang_argc = 0;
                    std::vector<const char*> clang_args = {};

                    std::transform(compilation_args.cbegin(), compilation_args.cend(), std::back_inserter(clang_args),
                                [&clang_argc](const std::string& s) {
                                    clang_argc++;
                                    return s.c_str();
                                });

                    const char** clang_argsv = (const char**)(clang_args.data());

                    for (int i = 0; i < clang_argc; i++) {
                        std::cout << clang_argsv[i] << "; ";
                    }
                    std::cout << std::endl;

                    auto clangOptsParser = CommonOptionsParser::create(clang_argc, clang_argsv, WashS2STCategory);
                    if (!clangOptsParser) {
                        llvm::errs() << clangOptsParser.takeError();
                        throw std::runtime_error("Error: Couldn't create Clang Options Parser");
                    }

                    CommonOptionsParser& passParser = clangOptsParser.get();
                    RefactoringTool passTool(passParser.getCompilations(), passParser.getSourcePathList());
                    ASTMatchRefactorer matcher(passTool.getReplacements());

                    std::vector<WashMatchCallback> matchCallbacks;
                    std::vector<std::variant<StatementMatcher*, DeclarationMatcher*>> matchers;
                    for (auto action : pass.actions()) {
                        matchCallbacks.emplace_back(action.getCallbackFn());
                        matchers.emplace_back(action.getMatcher());
                    }

                    for (size_t i = 0; i < pass.actions().size(); i++) {
                        WashMatchCallback* callback = &matchCallbacks.at(i);
                        std::visit([&matcher, callback](auto&& astmatch) { matcher.addMatcher(*astmatch, callback); },
                                matchers.at(i));
                    }

                    std::cout << "\tRunning Tool and Matchers" << std::endl;
                    int success = passTool.runAndSave(newFrontendActionFactory(&matcher).get());

                    if (success != 0) {
                        throw std::runtime_error("Pass " + std::to_string(passno) +
                                                " failed with tool showing non-zero exit code");
                    }
                }

                std::cout << "\tRunning Computation Steps" << std::endl;
                // Finally run the computations, or just run them if there were no files attached.
                for (auto comp : pass.computations()) {
                    comp.getComputeFn()(opts);
                }

                passno++;
            }

            return true;
        }

        void runRefactoring(const WashOptions& opts) { config::getConfigurationForImplementation(opts.impl).run(opts); }
    }

}