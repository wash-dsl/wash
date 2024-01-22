/**
 * @file reg_forces.hpp
 * @author james
 * @brief Defines some a basic matching to find force regsitrations and put them in some sets
 * @version 0.1
 * @date 2024-01-22
 * 
 * @copyright Copyright (c) 2024
 */
#pragma once

#include "common.hpp"

namespace wash {

    extern StatementMatcher addForceVectorMatcher;
    extern StatementMatcher addForceScalarMatcher;

    class RegisterForces {
    public:
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
                std::string name = forceName->getString().str();

                if (auto search = force_meta.find(name); search != force_meta.end()) {
                    FullSourceLoc othLoc = force_meta.at(name);
                    std::cerr << "Force already registered " << name << " at "
                            << othLoc.getSpellingLineNumber() << ":" << othLoc.getSpellingColumnNumber() 
                            << srcMgr->getFilename(othLoc).str() << std::endl;
                    return;
                }

                force_meta[name] = location;

                if (type == SCALAR) {
                    scalar_forces.insert(name);
                } else if (type == VECTOR) {
                    vector_forces.insert(name);
                }
            }
        };

        // Returns 0 success, 1 error, 2 some files not parsed
        static int checkRegisteredForces(ClangTool& Tool) {
            MatchFinder RegisterForceFinder;
            RegisterForcesCallback<SCALAR> scalarForceCallback;
            RegisterForcesCallback<VECTOR> vectorForceCallback;

            RegisterForceFinder.addMatcher(addForceVectorMatcher, &vectorForceCallback);
            RegisterForceFinder.addMatcher(addForceScalarMatcher, &scalarForceCallback);

            return Tool.run(newFrontendActionFactory(&RegisterForceFinder).get());
        }

    };

}