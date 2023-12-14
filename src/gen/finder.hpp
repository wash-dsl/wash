/**
 * @file finder.hpp
 * @author jamesm2w
 * @brief Find Wash Function is a small tool/plugin which scans 
 *  through program source code and reports a) wash function declarations
 *  and b) wash function calls
 * @version 0.1
 * @date 2023-12-14
 * 
 * @copyright Copyright (c) 2023
 */
#pragma once

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "iostream"

class FindWashFunctionVisitor : public clang::RecursiveASTVisitor<FindWashFunctionVisitor> {
private:
    clang::ASTContext *Context;

public:
    explicit FindWashFunctionVisitor(clang::ASTContext *Context) : Context(Context) {}
    bool VisitFunctionDecl(clang::FunctionDecl *Declaration);
    bool VisitCallExpr(clang::CallExpr *Expression);
};

class FindWashFunctionConsumer : public clang::ASTConsumer {
private:
    FindWashFunctionVisitor Visitor;

public:
    explicit FindWashFunctionConsumer(clang::ASTContext *Context) : Visitor(Context) {}
    virtual void HandleTranslationUnit(clang::ASTContext &Context);
};