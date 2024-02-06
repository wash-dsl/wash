#include "forces.hpp"

namespace wash {

namespace refactor {

namespace forces {

    DeclarationMatcher InsertForcesDefinitionMatcher = traverse(TK_IgnoreUnlessSpelledInSource, 
        cxxRecordDecl(hasName("_declare_force_vectors")).bind("decl")
    );

    void HandleInsertForcesDefinition(const MatchFinder::MatchResult &Result, Replacements& Replace) {
        const auto decl = Result.Nodes.getNodeAs<CXXRecordDecl>("decl");

        std::string replacementStr = "";

        for (auto vector_f : program_meta->vector_force_list) {
            replacementStr += "\nextern std::vector<SimulationVecT> vector_force_" + vector_f + ";";
        }

        for (auto scalar_f : program_meta->scalar_force_list) {
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

    std::string getForceDeclarationSource() {
        std::string output_str;

        for (auto scalar : program_meta->scalar_force_list) {
            output_str += "std::vector<double> scalar_force_" + scalar + ";\n";
        }

        for (auto vector : program_meta->vector_force_list) {
            output_str += "std::vector<SimulationVecT> vector_force_" + vector + ";\n";
        }

        return output_str;
    }

    std::string getForceInitialisationSource() {
        std::string output_str;

        for (auto scalar : program_meta->scalar_force_list) {
            output_str += "wash::scalar_force_" + scalar + " = std::vector<double>(particlec);\n";
        }

        for (auto vector : program_meta->vector_force_list) {
            output_str += "wash::vector_force_" + vector + " = std::vector<double>(particlec);\n";
        }

        return output_str;
    }
}

}

}