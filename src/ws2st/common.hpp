#pragma once

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/RefactoringCallbacks.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <iostream>
#include <fstream>
#include <unordered_set>
#include <string>
#include <any>

using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

namespace wash {
    enum class ForceType { SCALAR, VECTOR };

    struct WashProgramMeta {

        std::vector<std::string> scalar_force_list;
        std::vector<std::string> vector_force_list;
        std::unordered_map<std::string, FullSourceLoc> force_meta;

        std::vector<std::pair<std::string, std::string>> variable_list;
    };

    extern std::shared_ptr<WashProgramMeta> program_meta;

    class WashMatchCallback : public RefactoringCallback {
    private: 
        std::shared_ptr<WashProgramMeta> meta_info;
        std::function<void(const MatchFinder::MatchResult&)> callback_ptr;
    public:
        WashMatchCallback(std::shared_ptr<WashProgramMeta> meta_info, std::function<void (const MatchFinder::MatchResult &)> callback_ptr) : meta_info(meta_info), callback_ptr(callback_ptr) {}
        
        virtual void run (const MatchFinder::MatchResult &Result) {
            callback_ptr(Result);
        }
    };

    class WashRefactoringAction {
    private: 
        StatementMatcher matcher;
        WashMatchCallback callback;
    public: 
        WashRefactoringAction(StatementMatcher matcher, std::function<void (const MatchFinder::MatchResult &)> callback_ptr) : matcher(matcher), callback(program_meta, callback_ptr) {
            // matcher = matcher;
            // callback = WashMatchCallback(&program_meta, callback_ptr);
        }
        
        void apply (ASTMatchRefactorer& refactor) {
            refactor.addMatcher(matcher, &callback);
        }
    };
    
}

std::optional<std::string> getSourceText(ASTContext *ctx, SourceRange srcRange);