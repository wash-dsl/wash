/**
 * @file reg_forces.cpp
 * @author james
 * @brief defines some implementations for the static/global things used to find registered forces
 * @version 0.1
 * @date 2024-01-22
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "reg_forces.hpp"

namespace wash {

    int gatherSimulationInformation(RefactoringTool& Tool) {
        ASTMatchRefactorer finder(Tool.getReplacements());
        information::RegisterForcesCallback<ForceType::SCALAR> scalarForceCallback;
        information::RegisterForcesCallback<ForceType::VECTOR> vectorForceCallback;
        information::SimulationDimensionRefactor getDimensionCallback;

        finder.addMatcher(addForceVectorMatcher, &vectorForceCallback);
        finder.addMatcher(addForceScalarMatcher, &scalarForceCallback);
        finder.addMatcher(setSimulationDimensionMatcher, &getDimensionCallback);

        int code = Tool.runAndSave(newFrontendActionFactory(&finder).get());

        std::cout << "Gather Call Rewriting" << std::endl;
        for (auto repl : Tool.getReplacements()) {
            std::cout << repl.first << std::endl;
            for (auto rrepl : repl.second) {
                std::cout << "\t" << rrepl.toString() << std::endl;
            }
        }

        return code;
    }

    // Matches calls to add force vector inside the main function
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

    // Matches calls to add force scalar inside the main function
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

    StatementMatcher setSimulationDimensionMatcher = traverse(TK_IgnoreUnlessSpelledInSource, callExpr(
        hasAncestor(functionDecl(hasName("main"))),
        hasDescendant(
            declRefExpr(
                to(functionDecl(
                    hasName("wash::set_dimension")
                ))
            )
        ),
        hasArgument(0, ignoringImplicit( integerLiteral().bind("simDimension") ))
    ).bind("callExpr"));

    // define some implementations
    std::unordered_set<std::string> information::scalar_forces = {};
    std::unordered_set<std::string> information::vector_forces = {};
    uint64_t information::simulation_dimension = 0;

    std::unordered_map<std::string, FullSourceLoc> information::force_meta = {};

    template<ForceType type>
    void information::RegisterForcesCallback<type>::run(const MatchFinder::MatchResult &Result) {
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

        if (type == ForceType::SCALAR) {
            scalar_forces.insert(name);
        } else { // if (type == ForceType::VECTOR)
            vector_forces.insert(name);
        }

        auto Err = Replace.add(Replacement( 
            *Result.SourceManager, 
            CharSourceRange::getTokenRange(callExpr->getSourceRange()), 
            "" 
        ));

        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
        } else {
            std::cout << "Removed call to add force." <<  std::endl;
        }
    }

    void information::SimulationDimensionRefactor::run(const MatchFinder::MatchResult &Result)  {
        const auto call = Result.Nodes.getNodeAs<CallExpr>("callExpr");
        const auto value = Result.Nodes.getNodeAs<IntegerLiteral>("simDimension");

        if (!call || !value) {
            std::cerr << "Set simulation dimension call with no call or integer reference." << std::endl;
            return;
        }
        
        std::cout << "Picked up dimension " << *value->getValue().getRawData() << std::endl;
        simulation_dimension = *value->getValue().getRawData();

        auto Err = Replace.add(Replacement( 
            *Result.SourceManager, 
            CharSourceRange::getTokenRange(call->getSourceRange()), 
            "" 
        ));
        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
        } else {
            std::cout << "Removed call to set simulation dimension." <<  std::endl;
        }
    }
}