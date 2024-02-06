#include "forces.hpp"

namespace wash {

namespace refactor {

namespace forces {

    StatementMatcher AddForceVectorMatcher = traverse(TK_IgnoreUnlessSpelledInSource, callExpr(
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

    StatementMatcher AddForceScalarMatcher = traverse(TK_IgnoreUnlessSpelledInSource, callExpr(
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

    std::vector<std::string> scalar_force_list = {};
    std::vector<std::string> vector_force_list = {};
    std::unordered_map<std::string, FullSourceLoc> force_meta = {};

    template<ForceType type>
    void HandleRegisterForces(const MatchFinder::MatchResult &Result, Replacements& Replace) {
        const clang::CallExpr *callExpr = Result.Nodes.getNodeAs<clang::CallExpr>("callExpr");
        const clang::StringLiteral *forceName = Result.Nodes.getNodeAs<clang::StringLiteral>("forceName");

        if (!callExpr || !forceName) {
            std::cerr << "Match found without callExpr or forceName" << std::endl;
            throw std::runtime_error("Register match had no force name or call node");
        }

        FullSourceLoc location = Result.Context->getFullLoc(callExpr->getBeginLoc());
        std::string name = forceName->getString().str();

        if (auto search = program_meta->force_meta.find(name); search != program_meta->force_meta.end()) {
            FullSourceLoc othLoc = program_meta->force_meta.at(name);
            std::cerr << "Force already registered " << name << " at "
                    << othLoc.getSpellingLineNumber() << ":" << othLoc.getSpellingColumnNumber() 
                    << Result.Context->getSourceManager().getFilename(othLoc).str() << std::endl;
            return;
        }

        program_meta->force_meta[name] = location;
        if (type == ForceType::SCALAR) {
            program_meta->scalar_force_list.push_back(name);
        } else { // if (type == ForceType::VECTOR)
            program_meta->vector_force_list.push_back(name);
        }

        auto Err = Replace.add(Replacement( 
            *Result.SourceManager, 
            CharSourceRange::getTokenRange(callExpr->getSourceRange()), 
            "" 
        ));

        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
        } else {
            std::cout << "Removed the call to add force " << name <<  std::endl;
        }
    }
}

}

}