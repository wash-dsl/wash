/**
 * @file kernels.hpp
 * @author jamesm2w
 * @brief Defines the classes/functions used by the kernels plugin.
 *  This will find all kernel functions, and other things/symbols registered with wash.  
 * @version 0.1
 * @date 2023-12-18
 * 
 * @copyright Copyright (c) 2023
 */
#pragma once

#include <clang/AST/AST.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Sema/SemaConsumer.h>
#include <clang/Sema/Sema.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include <iostream>

namespace wash {

class WashKernelConsumer : public clang::SemaConsumer {
private:

    class ASTVisitor : public clang::RecursiveASTVisitor<ASTVisitor> {
    private:
        clang::Sema* SemaInst;

    public:
        ASTVisitor() {}
        
        void InitializeSema(clang::Sema& S) {
            SemaInst = &S;
        }

        void ForgetSema() {
            SemaInst = nullptr;
        }

        bool VisitFunctionDecl(clang::FunctionDecl *Declaration);
        bool VisitCallExpr(clang::CallExpr *Expression);
    };

    clang::Sema* SemaInst;
    ASTVisitor visitor;
public:
    WashKernelConsumer() {
        std::cout << "Wash Kernel Consumer Created" << std::endl;
    }

    void InitializeSema(clang::Sema& S) override {
        SemaInst = &S;
        visitor.InitializeSema(S);
    }

    void ForgetSema() override {
        SemaInst = nullptr;
        visitor.ForgetSema();
    }

    void HandleTranslationUnit(clang::ASTContext &Context) override;
    bool HandleTopLevelDecl(clang::DeclGroupRef DG) override;
};

class WashKernelsAction : 
#ifdef PLUGIN
    public clang::PluginASTAction
#else 
    public clang::ASTFrontendAction
#endif
{
public:
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &Compiler,
                                                                  llvm::StringRef InFile) override {
        return std::make_unique<WashKernelConsumer>();
    }

#ifdef PLUGIN
    bool ParseArgs(const clang::CompilerInstance &CI, const std::vector<std::string> &args) override {
        return true;
    }

    clang::PluginASTAction::ActionType getActionType() override { return AddBeforeMainAction; }
#endif
};

#ifdef PLUGIN
static clang::FrontendPluginRegistry::Add<WashKernelsAction> X("WashKernels", "Handle Wash Kernels");
#endif
}

#ifndef PLUGIN
int main(int argc, char **argv) {
    if (argc > 1) {
        clang::tooling::runToolOnCode(std::make_unique<wash::WashKernelsAction>(), argv[1]);
    } else {
        std::string buf;
        std::cin >> buf;
        clang::tooling::runToolOnCode(std::make_unique<wash::WashKernelsAction>(), buf);
    }
    return 0;
}
#endif