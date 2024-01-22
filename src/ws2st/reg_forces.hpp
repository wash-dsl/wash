#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <iostream>
#include <unordered_set>
#include <string>

using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

namespace wash {

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
    ).bind("callExpr"));

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
    ).bind("callExpr"));


    class RegisterForces {
    private:
        static std::unordered_set<std::string> scalar_forces;
        static std::unordered_set<std::string> vector_forces;
        static std::unordered_map<std::string, FullSourceLoc> force_meta;

        enum ForceType { SCALAR, VECTOR };

        template<ForceType type>
        class RegisterForcesCallback : public MatchFinder::MatchCallback {
        public:
            void run(const MatchFinder::MatchResult &Result) {

                SourceManager *srcMgr = Result.SourceManager;
                ASTContext *ctx = Result.Context;

                const clang::CallExpr *callExpr = Result.Nodes.getNodeAs<clang::CallExpr>("callExpr");
                const clang::StringLiteral *forceName = Result.Nodes.getNodeAs<clang::StringLiteral>("forceName");

                if (!callExpr || !forceName) {
                    std::cerr << "Match found without callExpr or forceName" << std::endl;
                    return;
                }

                FullSourceLoc location = ctx->getFullLoc(callExpr->getBeginLoc());
                std::string name = forceName->getStrDataAsChar();

                if (FullSourceLoc othLoc = force_meta.find(name)) {
                    std::cerr << "Force already registered " << name << " at "
                            << othLoc.getSpellingLineNumber() << ":" << othLoc.getSpellingColumnNumber() 
                            << srcMgr->getFilename(othLoc.getFileEntry()) << std::endl;
                    return;
                }

                force_meta.insert(name, location);

                if (type == SCALAR) {
                    scalar_forces.insert(name);
                } else if (type == VECTOR) {
                    vector_forces.insert(name);
                }
            }
        };

    public: 
        
        static std::pair<std::unordered_set<std::string>, std::unordered_set<std::string>> checkRegisteredForces(ClangTool& Tool) {
            MatchFinder RegisterForceFinder;
            RegisterForcesCallback<SCALAR> scalarForceCallback;
            RegisterForcesCallback<VECTOR> vectorForceCallback;
            RegisterForceFinder.addMatcher(addForceVectorMatcher, &vectorForceCallback);
            RegisterForceFinder.addMatcher(addForceScalarMatcher, &scalarForceCallback);
            // ForceFinder.addMatcher(anyCallInMain, &AnythingInMainPrinter);

            Tool.run(newFrontendActionFactory(&RegisterForceFinder).get());
            
            return {
                scalar_forces,
                vector_forces
            };
        }

    };

}