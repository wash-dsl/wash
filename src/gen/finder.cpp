/**
 * @file finder.cpp
 * @author jamesm2w
 * @brief Implements the Find Wash Function funtionality
 * @version 0.1
 * @date 2023-12-14
 *
 * @copyright Copyright (c) 2023
 */
#include "finder.hpp"

bool FindWashFunctionVisitor::VisitFunctionDecl(clang::FunctionDecl *Declaration) {
    if (Declaration->getQualifiedNameAsString().substr(0, 6) == "wash::") {
        clang::FullSourceLoc FullLocation = Context->getFullLoc(Declaration->getBeginLoc());
        clang::SourceManager &srcMgr = Context->getSourceManager();

        if (FullLocation.isValid()) {
            std::cout << "Found Wash Function Definition beginning at " << srcMgr.getFilename(FullLocation).data()
                      << ":" << FullLocation.getSpellingLineNumber() << ":" << FullLocation.getSpellingColumnNumber()
                      << std::endl;
        }
    }

    return true;
}

bool FindWashFunctionVisitor::VisitCallExpr(clang::CallExpr *Expression) {
    clang::FunctionDecl *Declaration = Expression->getDirectCallee();
    if (Declaration != nullptr) {
        if (Declaration->getQualifiedNameAsString().substr(0, 6) == "wash::") {
            clang::FullSourceLoc DeclLocation = Context->getFullLoc(Declaration->getBeginLoc());
            clang::FullSourceLoc CallLocation = Context->getFullLoc(Expression->getBeginLoc());

            clang::SourceManager &srcMgr = Context->getSourceManager();

            std::cout << "Found Wash Function Call (" << Declaration->getNameAsString() << ") beginning at ";
            std::cout << srcMgr.getFilename(CallLocation).data() << ":";
            if (CallLocation.isValid()) {
                std::cout << CallLocation.getSpellingLineNumber() << ":" << CallLocation.getSpellingColumnNumber();
            } else {
                std::cout << "(unkown location)";
            }

            std::cout << " calling function defined at ";
            std::cout << srcMgr.getFilename(DeclLocation).data() << ":";
            if (DeclLocation.isValid()) {
                std::cout << DeclLocation.getSpellingLineNumber() << ":" << DeclLocation.getSpellingColumnNumber();
            } else {
                std::cout << "(unknown location)";
            }
            std::cout << std::endl;
        }
    }

    return true;
}

void FindWashFunctionConsumer::HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
}