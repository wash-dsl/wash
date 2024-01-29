#include "forces.hpp"

namespace wash {

namespace refactor {

namespace forces {

    DeclarationMatcher InsertForcesDefinitionMatcher = traverse(TK_IgnoreUnlessSpelledInSource, 
        cxxRecordDecl(hasName("_declare_force_vectors")).bind("decl")
    );

    void HandleInsertForcesDefinition::run(const MatchFinder::MatchResult &Result) {
        const auto decl = Result.Nodes.getNodeAs<CXXRecordDecl>("decl");

        std::string replacementStr = "";

        for (auto vector_f : vector_force_list) {
            replacementStr += "\nextern std::vector<SimulationVecT> vector_force_" + vector_f + ";";
        }

        for (auto scalar_f : scalar_force_list) {
            replacementStr += "\nextern std::vector<double> scalar_force_" + scalar_f + ";";
        }

        auto Err = Replace.add(Replacement(
            *Result.SourceManager, CharSourceRange::getTokenRange(decl->getSourceRange()), replacementStr));

        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
        } else {
            std::cout << "Inserted forces definition" << std::endl;
        }
    }

    // TODO: Add the force declaration (w/out extern) / initialisation (with the particlec) generation here too
}

}

}