/**
 * @file kernels.cpp
 * @author jamesm2w
 * @brief Implementations for the kernel plugin/tool
 * @version 0.1
 * @date 2023-12-18
 * 
 * @copyright Copyright (c) 2023
 */

#include "kernels.hpp"

bool wash::WashKernelConsumer::ASTVisitor::VisitFunctionDecl(clang::FunctionDecl *Declaration) {
    if (Declaration->getQualifiedNameAsString().substr(0, 6) == "wash::") {
        clang::FullSourceLoc FullLocation = Declaration->getASTContext().getFullLoc(Declaration->getBeginLoc());
        clang::SourceManager &srcMgr = Declaration->getASTContext().getSourceManager();

        if (FullLocation.isValid()) {
            std::cout << "Found Wash Function Definition beginning at " << srcMgr.getFilename(FullLocation).data()
                      << ":" << FullLocation.getSpellingLineNumber() << ":" << FullLocation.getSpellingColumnNumber()
                      << std::endl;
        }
    }

    return true;
}

bool wash::WashKernelConsumer::ASTVisitor::VisitCallExpr(clang::CallExpr *Expression) {
    return true;
}

void wash::WashKernelConsumer::HandleTranslationUnit(clang::ASTContext &Context) {
    assert(Context.getTranslationUnitDecl() != nullptr);

    // SemaInst->PrintStats();

    // SemaInst->PrintContextStack();

    // clang::SourceManager &srcMgr = Context.getSourceManager();
    // clang::StringRef fileName = srcMgr.getFilename( Context.getFullLoc(Context.getTranslationUnitDecl()->getBeginLoc()) );

    // if (fileName.data() != nullptr) {
    //     std::cout << "Handle Translation Unit for " << fileName.data() << std::endl;
    // } else {
    //     std::cout << "Handle Translation Unit (bad filename?)" << std::endl;
    // }
    std::cout << std::endl;
    // visitor.TraverseDecl(Context.getTranslationUnitDecl());
}

bool wash::WashKernelConsumer::HandleTopLevelDecl(clang::DeclGroupRef DG) {
    // clang::SourceManager &srcMgr = SemaInst->getASTContext().getSourceManager();

    // if (DG.isSingleDecl()) {
    //     std::cout << "Handle Top Level Decl (single) from " << srcMgr.getFilename(DG.getSingleDecl()->getBeginLoc()).data() << std::endl;
    // } else {
    //     std::cout << "Handle Top Level Decl size " << DG.getDeclGroup().size() << " from " << srcMgr.getFilename(DG.getDeclGroup()[0]->getBeginLoc()).data() << std::endl;
    // }

    return true;
}