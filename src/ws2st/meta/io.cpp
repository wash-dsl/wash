#include "meta.hpp"
namespace ws2st {

namespace refactor {

namespace meta {

    std::string writeCopySimData(bool cornerstone_vectors = false) {
        auto scalarForceNames = program_meta->scalar_force_list;
        scalarForceNames.push_back("mass");
        scalarForceNames.push_back("density");
        scalarForceNames.push_back("smoothing_length");
        if (cornerstone_vectors) {
            scalarForceNames.push_back("id");
        }
        
        auto vectorForceNames = program_meta->vector_force_list;
        vectorForceNames.push_back("pos");
        vectorForceNames.push_back("vel");
        vectorForceNames.push_back("acc");
        
        std::string output_string = "namespace io {\n\tSimulationData copy_simulation_data() {\n\t";

        // write dim and label vectors
        output_string += "std::vector<unsigned short> dims {"; 
        for (auto scalar : scalarForceNames) {
            output_string += "1, ";
        }
        for (auto vector : vectorForceNames) {
            output_string += std::to_string(program_meta->simulation_dimension) + ", ";
        }
        output_string += "};\n\t";

        output_string += "std::vector<std::string> labels {"; 
        for (auto scalar : scalarForceNames) {
            output_string += "\"" + scalar + "\", ";
        }
        for (auto vector : vectorForceNames) {
            output_string += "\"" + vector + "\", ";
        }
        output_string += "};\n\t";
        output_string += "size_t particle_count = wash::scalar_force_mass.size();\n\t";
        size_t particle_width = scalarForceNames.size() + (program_meta->simulation_dimension) * vectorForceNames.size();
        output_string += "size_t particle_width = " + std::to_string(particle_width) + ";\n\t";
        output_string += "std::vector<double> sim_data(particle_width * particle_count);\n\t";
        output_string += "for (size_t i = 0; i < particle_count; i++) {\n\t";
        size_t force_index = 0;

        // write out the copied data
        for (auto& scalar : scalarForceNames) {
            output_string += "sim_data[i*" + std::to_string(particle_width) + " + "+ std::to_string(force_index) +"] = wash::scalar_force_" + scalar + "[i];\n\t";
            // output_string += "force_index += 1;\n\t"; 
            force_index += 1;
        }

        for (auto& vector : vectorForceNames) {
            for (int i = 0; i < program_meta->simulation_dimension; i++) {
                if (cornerstone_vectors) {
                    output_string += "sim_data[i*" + std::to_string(particle_width) + " + "+ std::to_string(force_index) +" + "+std::to_string(i)+"] = wash::vector_force_" + vector + "_" + std::to_string(i) + "[i];\n\t";
                } else {
                    output_string += "sim_data[i*" + std::to_string(particle_width) + " + "+ std::to_string(force_index) +" + "+std::to_string(i)+"] = wash::vector_force_" + vector + "[i]["+std::to_string(i)+"];\n\t";
                }
            }
            // output_string += "force_index += " + std::to_string(program_meta->simulation_dimension) + ";\n\t";
            force_index += program_meta->simulation_dimension;
        }
        output_string += "}\n\t";
        output_string += "return SimulationData {.particle_count = particle_count, .data = sim_data, .labels = labels, .dim = dims };\n";
        output_string += "\t}\n}\n";
        return output_string;
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

        output_str += writeCopySimData(false);

        output_str += "std::vector<double> copy_variables() {\n\t";
        output_str += "return {";
        for (auto variable : program_meta->variable_list) {
            output_str += "wash::variable_" + variable + ", ";
        }
        output_str += "}; }\n";

        output_str += "std::vector<std::string> get_variables_names() {\n\t";
        output_str += "return {";
        for (auto variable : program_meta->variable_list) {
            output_str += "\"" + variable + "\", ";
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

    void DefineForceAccessFnsWithCornerstone(const MatchFinder::MatchResult& Result, Replacements& Replace) {
        const auto *decl = Result.Nodes.getNodeAs<CXXRecordDecl>("decl");
        if (!decl) {
            throw std::runtime_error("Missing decl");
        }
        std::string output_str = "";

        output_str += writeCopySimData(true);

        output_str += "std::vector<double> copy_variables() {\n\t";
        output_str += "return {";
        for (auto variable : program_meta->variable_list) {
            output_str += "wash::variable_" + variable + ", ";
        }
        output_str += "}; }\n";

        output_str += "std::vector<std::string> get_variables_names() {\n\t";
        output_str += "return {";
        for (auto variable : program_meta->variable_list) {
            output_str += "\"" + variable + "\", ";
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