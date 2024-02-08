#include "meta.hpp"

#include <utility>

namespace wash {

namespace refactor {

namespace meta {

    DeclarationMatcher DefineForceAccessFnMatcher = traverse(TK_IgnoreUnlessSpelledInSource, 
        cxxRecordDecl(hasName("_define_force_access_fn")).bind("decl")
    );

    void DefineForceAccessFns(const MatchFinder::MatchResult& Result, Replacements& Replace) {
        const auto *decl = Result.Nodes.getNodeAs<CXXRecordDecl>("decl");
        if (!decl) {
            throw std::runtime_error("Missing decl");
        }
        std::string output_str = "";
        for (auto type :  std::vector<std::string> {"scalar", "vector"}) {
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

            output_str += "const std::vector<std::vector<" + typeName + ">*> get_force_" + type + "s() {\n\t";
            output_str += "return {";
            for (auto force : forcesNames) {
                output_str += "&wash::" + type + "_force_" + force + ", ";
            }
            output_str += "}; }\n";

            output_str += "const std::vector<std::string> get_force_" + type + "s_names() {\n\t";
            output_str += "return {";
            for (auto force : forcesNames) {
                output_str += "\"" + force + "\", ";
            }
            output_str += "};\n}";

            output_str += "\n";
        }

        output_str += "const std::vector<double*> get_variables() {\n\t";
        output_str += "return {";
        for (auto variable : program_meta->variable_list) {
            output_str += "&wash::variable_" + variable.first + ", ";
        }
        output_str += "}; }\n";

        output_str += "const std::vector<std::string> get_variables_names() {\n\t";
        output_str += "return {";
        for (auto variable : program_meta->variable_list) {
            output_str += "\"" + variable.first + "\", ";
        }
        output_str += "}; }\n";

        auto Err = Replace.add(Replacement(
            *Result.SourceManager, CharSourceRange::getTokenRange(decl->getSourceRange()), output_str));
        if (Err) {
            llvm::errs() << "Error performing replacement " << std::move(Err) << "\n";
        } else {
            std::cout << "Rewrote function to [[get forces arrays]]" << std::endl;
        }
    }

    // DeclarationMatcher GetForceScalarsDeclMatcher = traverse(TK_IgnoreUnlessSpelledInSource, functionDecl(
    //     hasName("wash::get_force_scalars")
    // ).bind("funcDecl"));

    // DeclarationMatcher GetForceVectorsDeclMatcher = traverse(TK_IgnoreUnlessSpelledInSource, functionDecl(
    //     hasName("wash::get_force_vectors")
    // ).bind("funcDecl"));

    // DeclarationMatcher GetForceScalarsNamesDeclMatcher = traverse(TK_IgnoreUnlessSpelledInSource, functionDecl(
    //     hasName("wash::get_force_scalars_names")
    // ).bind("funcDecl"));

    // DeclarationMatcher GetForceVectorsNamesDeclMatcher = traverse(TK_IgnoreUnlessSpelledInSource, functionDecl(
    //     hasName("wash::get_force_vectors_names")
    // ).bind("funcDecl"));

    // WashCallbackFn handleGetForceVectors = &meta::HandleGetForceTypeDecl<ForceType::VECTOR>;
    // WashCallbackFn handleGetForceScalars = &meta::HandleGetForceTypeDecl<ForceType::SCALAR>;
    // WashCallbackFn handleGetForceVectorsNames = &meta::HandleGetForceTypeNamesDecl<ForceType::VECTOR>;
    // WashCallbackFn handleGetForceScalarsNames = &meta::HandleGetForceTypeNamesDecl<ForceType::SCALAR>;

    // template<ForceType type>
    // void HandleGetForceTypeDecl(const MatchFinder::MatchResult& Result, Replacements& Replace) {
    //     const auto *funcDecl = Result.Nodes.getNodeAs<FunctionDecl>("funcDecl");

    //     if (!funcDecl) {
    //         std::cerr << "Matched node missing function declaration" << std::endl;
    //         throw std::runtime_error("Missing function decl");
    //     }

    //     const std::string forceTypeName = (type == ForceType::SCALAR) ? "scalar" : "vector";
    //     const std::string dataTypeName = (type == ForceType::SCALAR) ? "double" : "SimulationVecT";
    //     const std::vector<std::string> forcesVector = (type == ForceType::SCALAR) ? program_meta->scalar_force_list : program_meta->vector_force_list;

    //     const bool hasBody = funcDecl->doesThisDeclarationHaveABody();

    //     std::string replacement_str;
    //     const size_t num_forces = forcesVector.size();

    //     replacement_str += "const std::array<std::vector<" + dataTypeName + ">*," + std::to_string(num_forces) + "> get_force_" + forceTypeName + "s()";

    //     if (hasBody) { // This function specifically has a body which we should replace as well
    //         replacement_str += "{ return {";
    //         for (auto force : forcesVector) {
    //             replacement_str += "&wash::" + forceTypeName + "_force_" + force + ", ";
    //         }
    //         replacement_str += "}; }";
    //     } else { // This function doesn't have a body we only need to replace the type
    //         replacement_str += ";";
    //     }

    //     auto Err = Replace.add(Replacement(
    //         *Result.SourceManager, CharSourceRange::getTokenRange(funcDecl->getSourceRange()), replacement_str));
    //     if (Err) {
    //         llvm::errs() << "Error performing replacement " << std::move(Err) << "\n";
    //     } else {
    //         std::cout << "Rewrote function to [[get forces arrays]] body: " << hasBody << std::endl;
    //     }
    // }

    // template<ForceType type>
    // void HandleGetForceTypeNamesDecl(const MatchFinder::MatchResult& Result, Replacements& Replace) {
    //     const auto *funcDecl = Result.Nodes.getNodeAs<FunctionDecl>("funcDecl");

    //     if (!funcDecl) {
    //         std::cerr << "Matched node missing function declaration" << std::endl;
    //         throw std::runtime_error("Missing function decl");
    //     }

    //     // const std::string forceTypeName = (type == ForceType::SCALAR) ? "scalar" : "vector";
    //     // const std::string dataTypeName = (type == ForceType::SCALAR) ? "double" : "SimulationVecT";
    //     const std::vector<std::string> forcesVector = (type == ForceType::SCALAR) ? program_meta->scalar_force_list : program_meta->vector_force_list;

    //     const bool hasBody = funcDecl->doesThisDeclarationHaveABody();
    //     if (!hasBody) { // Don't need to replace anything on the decl
    //         return;
    //     }
    
    //     const auto* body = funcDecl->getBody();
    //     if (!body) {
    //         std::cerr << "No body" << std::endl;
    //         throw std::runtime_error("Missing function body");
    //     }

    //     std::string replacement_str = "{ return {";
    //     for (auto force : forcesVector) {
    //         replacement_str += "\"" + force + "\", ";
    //     }
    //     replacement_str += "}; }";

    //     auto Err = Replace.add(Replacement(
    //         *Result.SourceManager, CharSourceRange::getTokenRange(body->getSourceRange()), replacement_str));
    //     if (Err) {
    //         llvm::errs() << "Error performing replacement " << std::move(Err) << "\n";
    //     } else {
    //         std::cout << "Rewrote function to [[get forces names]]" << std::endl;
    //     }
    // }

    // StatementMatcher ParticleForGetForceVectors = traverse(TK_IgnoreUnlessSpelledInSource, cxxForRangeStmt(
    //     hasBody(compoundStmt().bind("loopBody")),
    //     hasLoopVariable(varDecl().bind("initVariable")),
    //     hasRangeInit(cxxMemberCallExpr(
    //         on(hasType(cxxRecordDecl(hasName("Particle")))),
    //         callee(cxxMethodDecl(hasName("get_force_vectors")))
    //     ).bind("callExpr"))
    // ).bind("forLoop"));

    // void HandleGetForceVectors(const MatchFinder::MatchResult& Result, Replacements& Replace) {
    //     const auto *forLoop = Result.Nodes.getNodeAs<CXXForRangeStmt>("forLoop");
    //     const auto * loopBody = Result.Nodes.getNodeAs<CompoundStmt>("loopBody");
    //     const auto * initVariable = Result.Nodes.getNodeAs<VarDecl>("initVariable");
    //     const auto * callExpr = Result.Nodes.getNodeAs<CXXMemberCallExpr>("callExpr");

    //     if (!forLoop || !loopBody || !initVariable || !callExpr) {
    //         std::cout << "Matched node missing critical nodes" << std::endl;
    //         throw std::runtime_error("Force Vectors missing things");
    //     }

    //     const Expr *objectExpr = callExpr->getImplicitObjectArgument();
    //     std::string objectCodeStr = getSourceText(Result.Context, objectExpr->getSourceRange()).value();
        
    //     std::string varName = initVariable->getNameAsString();

    //     std::string output_str = "";
    //     for (auto vector_force : program_meta->vector_force_list) {
    //         std::string loopBodySrc = getSourceText(Result.Context, loopBody->getSourceRange()).value();
    //         std::string varReplacement = objectCodeStr + ".get_force_vector(\"" + vector_force + "\")";

    //         size_t pos = loopBodySrc.find(varName);
    //         while (pos != std::string::npos) {
    //             loopBodySrc.replace(pos, varName.length(), varReplacement);
    //             pos = loopBodySrc.find(varName, pos + varReplacement.length());
    //         }

    //         output_str += "\n" + loopBodySrc + "// Finished " + varName + "; " + objectCodeStr + "; " + vector_force + "\n";
    //     }

    //     auto Err = Replace.add(Replacement(
    //         *Result.SourceManager, CharSourceRange::getTokenRange(forLoop->getSourceRange()), output_str));
    //     if (Err) {
    //         llvm::errs() << "Error performing replacement " << std::move(Err) << "\n";
    //     } else {
    //         std::cout << "Rewrote Particle Get Force Vectors Loop" << std::endl;
    //     }
    // }

    // // StatementMatcher SimulationForGetForceVectors;

    // void HandleSimulationForGetForceVectors(const MatchFinder::MatchResult& Result, Replacements& Replace) {

    // }

}

}

}