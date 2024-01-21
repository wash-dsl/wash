// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <iostream>

using namespace clang::tooling;
using namespace llvm;

using namespace clang;
using namespace clang::ast_matchers;

StatementMatcher LoopMatcher =
    forStmt(hasLoopInit(declStmt(hasSingleDecl(varDecl(
        hasInitializer(integerLiteral(equals(0)))
    ))))).bind("forLoop");

StatementMatcher addForceVectorMatcher = traverse(TK_IgnoreUnlessSpelledInSource, callExpr(
    hasAncestor(functionDecl(hasName("main"))),
    hasDescendant(
        declRefExpr(
            to(functionDecl(
                hasName("wash::add_force_vector")
            ))
        )
    ),
    hasArgument(0, ignoringImplicit( stringLiteral().bind("forceName") ))
).bind("call"));

StatementMatcher addForceScalarMatcher = traverse(TK_IgnoreUnlessSpelledInSource, callExpr(
    hasAncestor(functionDecl(hasName("main"))),
    hasDescendant(
        declRefExpr(
            to(functionDecl(
                hasName("wash::add_force_scalar")
            ))
        )
    ),
    hasArgument(0, ignoringImplicit( stringLiteral().bind("forceName") ))
).bind("call"));

StatementMatcher anyCallInMain = traverse(TK_IgnoreUnlessSpelledInSource, callExpr(
    hasAncestor(functionDecl(hasName("main"))),
    hasDescendant(
        declRefExpr(
            to(functionDecl(
                hasName("wash::add_force_vector")
            ))
        )
    ),
    hasArgument(0, ignoringImplicit( stringLiteral().bind("forceName") ))
).bind("thecall"));

class AnythingInMainPrinter : public MatchFinder::MatchCallback {
public:
    void run(const MatchFinder::MatchResult &Result) {
        std::cout << "got a match" << std::endl;

        if (const Stmt *S = Result.Nodes.getNodeAs<Stmt>("thecall")) {
            S->dump();
        }
    }
};

class LoopPrinter : public MatchFinder::MatchCallback {
public:
    virtual void run(const MatchFinder::MatchResult &Result) {
        if (const ForStmt *FS = Result.Nodes.getNodeAs<clang::ForStmt>("forLoop"))
            FS->dump();
    }
};

class ForcePrinter : public MatchFinder::MatchCallback {
public:
    virtual void run(const MatchFinder::MatchResult &Result) {
        std::cout << "got force definition" << std::endl;

        const clang::CallExpr *call = Result.Nodes.getNodeAs<clang::CallExpr>("call");
        SourceManager *srcMgr = Result.SourceManager;

        if (call) {
            FullSourceLoc fullLocation = Result.Context->getFullLoc(call->getBeginLoc());
            std::cout << "call at "; // << std::endl;
            std::cout << "location: " << srcMgr->getFilename(fullLocation).data()
                      << ":" << fullLocation.getSpellingLineNumber() << ":" << fullLocation.getSpellingColumnNumber()
                      << std::endl;
        }

        const clang::StringLiteral *argument = Result.Nodes.getNodeAs<clang::StringLiteral>("forceName");
        if (argument) {
            std::cout << "\tforce name " << argument->getBytes().data() << std::endl;
        }
    }
};

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory WashS2STCategory("Wash S2S Translator");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nThis is the WaSH source-to-source translator tool.\n");

int main(int argc, const char **argv) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, WashS2STCategory);
    if (!ExpectedParser) {
        // Fail gracefully for unsupported options.
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }

    CommonOptionsParser& OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(),
    OptionsParser.getSourcePathList());

    // LoopPrinter LoopPrinter;
    // MatchFinder LoopFinder;
    // LoopFinder.addMatcher(LoopMatcher, &LoopPrinter);

    // Tool.run(newFrontendActionFactory(&LoopFinder).get());

    ForcePrinter ForcePrinter;
    AnythingInMainPrinter AnythingInMainPrinter;
    MatchFinder ForceFinder;
    ForceFinder.addMatcher(addForceVectorMatcher, &ForcePrinter);
    ForceFinder.addMatcher(addForceScalarMatcher, &ForcePrinter);
    // ForceFinder.addMatcher(anyCallInMain, &AnythingInMainPrinter);

    return Tool.run(newFrontendActionFactory(&ForceFinder).get());
}
