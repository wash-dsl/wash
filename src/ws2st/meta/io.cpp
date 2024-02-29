#include "meta.hpp"
namespace ws2st {

namespace refactor {

namespace meta {

    /**
     * @brief Write functions to generate the access functions for the scalar/vector force data and names
     * 
     * @param type 
     * @return std::string 
     */
    std::string forceTypeAccessFns(std::string type) {
        std::string output_str;
        std::string typeName;
        std::vector<std::string> forcesNames;

        if (type == "scalar") {
            typeName = "double";
            forcesNames = program_meta->scalar_force_list;
        } else {
            typeName = "SimulationVecT";
            forcesNames = program_meta->vector_force_list;
        }

        const size_t num_forces = forcesNames.size();

        // TODO: look at std::reference_wrapper in future here, maybe?
        output_str += "std::vector<std::vector<" + typeName + ">*> get_force_" + type + "s() {\n\t";
        output_str += "return {";
        for (auto force : forcesNames) {
            output_str += "&wash::" + type + "_force_" + force + ", ";
        }
        output_str += "}; }\n";

        output_str += "std::vector<std::string> get_force_" + type + "s_names() {\n\t";
        output_str += "return {";
        for (auto force : forcesNames) {
            output_str += "\"" + force + "\", ";
        }
        output_str += "};\n}";

        output_str += "\n";
        return output_str;
    }

    DeclarationMatcher DefineForceAccessFnMatcher = traverse(TK_IgnoreUnlessSpelledInSource, 
        cxxRecordDecl(hasName("_define_force_access_fn")).bind("decl")
    );

    void DefineForceAccessFns(const MatchFinder::MatchResult& Result, Replacements& Replace) {
        const auto *decl = Result.Nodes.getNodeAs<CXXRecordDecl>("decl");
        if (!decl) {
            throw std::runtime_error("Missing decl");
        }
        std::string output_str = "";
        output_str += forceTypeAccessFns("scalar");
        output_str += forceTypeAccessFns("vector");

        output_str += "std::vector<double*> get_variables() {\n\t";
        output_str += "return {";
        for (auto variable : program_meta->variable_list) {
            output_str += "&wash::variable_" + variable.first + ", ";
        }
        output_str += "}; }\n";

        output_str += "std::vector<std::string> get_variables_names() {\n\t";
        output_str += "return {";
        for (auto variable : program_meta->variable_list) {
            output_str += "\"" + variable.first + "\", ";
        }
        output_str += "}; }\n";

        auto Err = Replace.add(Replacement(
            *Result.SourceManager, CharSourceRange::getTokenRange(decl->getSourceRange()), output_str));
        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
            throw std::runtime_error("Error handling a match callback.");
        } else {
            std::cout << "Rewrote function to [[get forces arrays]]" << std::endl;
        }
    }

    

}

}

}