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

namespace wash {

    std::optional<std::string> getSourceText(ASTContext* ctx, SourceRange srcRange) {
        const auto &SM = ctx->getSourceManager();
        const auto langOpts = ctx->getLangOpts();
        if (srcRange.isValid()) {
            auto code = Lexer::getSourceText(CharSourceRange::getTokenRange(srcRange), SM, langOpts);
            return code.str();
        } else {
            return std::nullopt;
        }
    }

    int forceNameRewriting(RefactoringTool& Tool) {
        ASTMatchRefactorer finder(Tool.getReplacements());
        refactoring::GetForceScalarRefactor getForcesScalar;

        finder.addMatcher(getForceScalarMatcher, &getForcesScalar);

        int code = Tool.runAndSave(newFrontendActionFactory(&finder).get());

        for (auto repl : Tool.getReplacements()) {
            std::cout << repl.first << std::endl;
            for (auto rrepl : repl.second) {
                std::cout << "\t" << rrepl.toString() << std::endl;
            }
        }

        if (code != 0) {
            std::cout << "error running tool" << std::endl;
            return code;
        }

        // std::cout << "time to write out" << std::endl;
        // writeOutReplacements(Tool);
        return 0;
    }

    StatementMatcher getForceScalarMatcher = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        on(hasType(cxxRecordDecl(hasName("Particle")))),
        callee(cxxMethodDecl(hasName("get_force_scalar"))),
        hasArgument(0, ignoringImplicit(stringLiteral().bind("forceName")))
    ).bind("callExpr"));

    StatementMatcher getForceVectorMatcher = callExpr();

    StatementMatcher setForceScalarMatcher = callExpr();
    StatementMatcher setForceVectorMatcher = callExpr();

    void writeOutReplacements(RefactoringTool& tool) {
        LangOptions DefaultLangOptions;
        IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
        TextDiagnosticPrinter DiagnosticPrinter(llvm::errs(), &*DiagOpts);
        DiagnosticsEngine Diagnostics(
            IntrusiveRefCntPtr<DiagnosticIDs>(new DiagnosticIDs()),
            &*DiagOpts, &DiagnosticPrinter, false);
        SourceManager Sources(Diagnostics, tool.getFiles());

        Rewriter rewrite (Sources, DefaultLangOptions);

        clang::tooling::formatAndApplyAllReplacements(tool.getReplacements(), rewrite, "Google");
        std::cout << "finished formatting" << std::endl;

        if (rewrite.overwriteChangedFiles()) {
            std::cout << "error writing files :(" << std::endl;
        }

        // for (auto I = rewrite.buffer_begin(), E = rewrite.buffer_end(); I != E; I++) {
        //     std::string filename = Sources.getFileEntryForID(I->first)->getName().str();
        //     std::error_code ec;
        //     llvm::raw_fd_ostream outFile { llvm::StringRef(filename), ec, llvm::sys::fs::OF_Text };
        //     std::cout << "writing out " << filename << std::endl;;
        //     I->second.write(outFile);
        // }
    }

}