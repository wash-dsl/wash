#include "ws2st.hpp"

std::unique_ptr<wash::WashProgramMeta> program_meta;
    
std::optional<std::string> getSourceText(ASTContext* ctx, SourceRange srcRange) {
    const auto &SM = ctx->getSourceManager();
    const auto langOpts = ctx->getLangOpts();
    if (srcRange.isValid()) {
        auto code = Lexer::getSourceText(CharSourceRange::getTokenRange(srcRange), SM, langOpts);
        return code.str();
    }

    return std::nullopt;
}

int main(int argc, const char** argv) {
    // argv == wash/old --> copy over the src and original wash backend no changes made
    //      == wash/isb --> copy over the src and new wisb backend and do the enum changes ets
    //      == openmp   --> add in linking with OpenMP

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " /path/to/source -- [other compiler options]" << std::endl;
        return 1;
    }

    std::vector<std::filesystem::path> files = wash::files::copy_wash_source(argv[1]);
    std::vector<std::string> new_args;

    new_args.push_back((std::string)argv[0]);

    for (const auto& path : files) {
        new_args.push_back(path);
    }

    for (int i = 2; i < argc; i++) {
        new_args.push_back((std::string)argv[i]);
    }

    wash::refactor::runRefactoringStages(new_args);

    std::cout << "finished refactoring" << std::endl;

    write_particle_initialiser((std::string) "build/tmp/" + wash::files::app_str + "/particle_data.cpp",
                               program_meta->scalar_force_list, program_meta->vector_force_list);

    return 0;
}

void write_particle_initialiser(std::string path, std::unordered_set<std::string> scalar_f, std::unordered_set<std::string> vector_f) {

    std::string output_str = "#include \"particle_data.hpp\" \n"
    "namespace wash {\n"
    "std::vector<SimulationVecT>* vector_force_pos;\n"
    "std::vector<SimulationVecT>* vector_force_vel;\n"
    "std::vector<SimulationVecT>* vector_force_acc;\n"
    "std::vector<double>* scalar_force_mass;\n"
    "std::vector<double>* scalar_force_density;\n"
    "std::vector<double>* scalar_force_smoothing_length;\n";

    for (auto scalar : scalar_f) {
        output_str += "std::vector<double>* scalar_force_" + scalar + ";\n";
    }

    for (auto vector : vector_f) {
        output_str += "std::vector<SimulationVecT>* vector_force_" + vector + ";\n";
    }

    output_str += wash::refactor::variables::getVariableDeclarationSource();    

    output_str += "    void _initialise_particle_data(size_t particlec) {\n"
    "        wash::vector_force_pos = new std::vector<SimulationVecT>(particlec);\n"
    "        wash::vector_force_vel = new std::vector<SimulationVecT>(particlec);\n"
    "        wash::vector_force_acc = new std::vector<SimulationVecT>(particlec);\n"
    "        wash::scalar_force_mass = new std::vector<double>(particlec);\n"
    "        wash::scalar_force_density = new std::vector<double>(particlec);\n"
    "        wash::scalar_force_smoothing_length = new std::vector<double>(particlec);\n";

    for (auto scalar : scalar_f) {
        output_str += "wash::scalar_force_" + scalar + " = new std::vector<double>(particlec);\n";
    }

    for (auto vector : vector_f) {
        output_str += "wash::vector_force_" + vector + " = new std::vector<SimulationVecT>(particlec);\n";
    }
    
    output_str += " } }";

    std::ios_base::openmode mode = std::ofstream::out;
    std::ofstream outfile (path.c_str(), mode);

    outfile << output_str.c_str() << std::endl;

    outfile.close();
}