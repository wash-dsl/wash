#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "iostream"

class FindWashFunctionVisitor : public clang::RecursiveASTVisitor<FindWashFunctionVisitor> {
public:
    explicit FindWashFunctionVisitor(clang::ASTContext *Context) : Context(Context) {}

    bool VisitFunctionDecl(clang::FunctionDecl *Declaration) {
        if (Declaration->getQualifiedNameAsString().substr(0, 6) == "wash::") {
            clang::FullSourceLoc FullLocation = Context->getFullLoc(Declaration->getBeginLoc());

            if (FullLocation.isValid()) {
                std::cout << "Found Wash Function Definition beginning at " << FullLocation.getSpellingLineNumber()
                          << ":" << FullLocation.getSpellingColumnNumber() << std::endl;
            }
        }

        return true;
    }

    bool VisitCallExpr(clang::CallExpr *Expression) {
        clang::FunctionDecl *Declaration = Expression->getDirectCallee();
        if (Declaration != nullptr) {
            if (Declaration->getQualifiedNameAsString().substr(0, 6) == "wash::") {
                clang::FullSourceLoc DeclLocation = Context->getFullLoc(Declaration->getBeginLoc());
                clang::FullSourceLoc CallLocation = Context->getFullLoc(Expression->getBeginLoc());

                std::cout << "Found Wash Function Call (" << Declaration->getNameAsString() << ") beginning at ";
                if (CallLocation.isValid()) {
                    std::cout << CallLocation.getSpellingLineNumber() << ":" << CallLocation.getSpellingColumnNumber();
                } else {
                    std::cout << "(unkown location)";
                }

                std::cout << " calling function defined at ";
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

private:
    clang::ASTContext *Context;
};

class FindWashFunctionConsumer : public clang::ASTConsumer {
public:
    explicit FindWashFunctionConsumer(clang::ASTContext *Context) : Visitor(Context) {}

    virtual void HandleTranslationUnit(clang::ASTContext &Context) {
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }

private:
    FindWashFunctionVisitor Visitor;
};

class FindWashFunctionAction : public clang::ASTFrontendAction {
public:
    virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &Compiler,
                                                                  llvm::StringRef InFile) {
        return std::make_unique<FindWashFunctionConsumer>(&Compiler.getASTContext());
    }
};

int main(int argc, char **argv) {
    if (argc > 1) {
        clang::tooling::runToolOnCode(std::make_unique<FindWashFunctionAction>(), argv[1]);
    } else {
        std::string buf;
        std::cin >> buf;
        clang::tooling::runToolOnCode(std::make_unique<FindWashFunctionAction>(), buf);
    }
}