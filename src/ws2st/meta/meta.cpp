#include "meta.hpp"

namespace wash {

namespace refactor {

namespace meta {

    StatementMatcher SetDimensionMatcher = traverse(TK_IgnoreUnlessSpelledInSource, callExpr(
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

    void HandleSetDimension(const MatchFinder::MatchResult& Result, Replacements& Replace) {
        const auto call = Result.Nodes.getNodeAs<CallExpr>("callExpr");
        const auto value = Result.Nodes.getNodeAs<IntegerLiteral>("simDimension");

        if (!call || !value) {
            std::cerr << "Set simulation dimension call with no call or integer reference." << std::endl;
            return;
        }
        
        std::cout << "Picked up dimension " << *value->getValue().getRawData() << std::endl;
        program_meta->simulation_dimension = *value->getValue().getRawData();

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

    DeclarationMatcher SimulationVecTMatcher = traverse(TK_IgnoreUnlessSpelledInSource, typedefNameDecl(
        hasName("SimulationVecT")
    ).bind("typedef"));

    void HandleSimulationVecTMatcher(const MatchFinder::MatchResult& Result, Replacements& Replace) {
        const auto typedefdecl = Result.Nodes.getNodeAs<TypedefNameDecl>("typedef");

        if (!typedefdecl) {
            std::cerr << "No typedef found" << std::endl;
            return;
        }

        std::string replacement = "using SimulationVecT = Vec<double, " + std::to_string(program_meta->simulation_dimension) + ">;";

        auto Err = Replace.add(Replacement(
            *Result.SourceManager, CharSourceRange::getTokenRange(typedefdecl->getSourceRange()), replacement));

        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
        } else {
            std::cout << "Updated SimulationVecT to use " << program_meta->simulation_dimension << " dimensions" << std::endl;
        }

    }

}

}

}