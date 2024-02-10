#include "ws2st.hpp"

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nThis is the WaSH source-to-source translator tool.\n");

std::optional<std::string> getSourceText(ASTContext* ctx, SourceRange srcRange) {
    const auto& SM = ctx->getSourceManager();
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

    size_t doubledashidx = -1;
    for (int ii = 0; ii < new_args.size(); ii++) {
        if (new_args[ii] == "--") {
            doubledashidx = ii;
        }
    }

    if (doubledashidx == -1) {
        new_args.push_back("--");
        doubledashidx = new_args.size() - 1;
    }

    new_args.push_back("-I/usr/lib64/clang/16/include");
    new_args.push_back("-std=c++17");
    new_args.push_back("-DWASH_HDF5_SUPPORT");
    new_args.push_back("-fopenmp");

    wash::refactor::runRefactoringStages(new_args);

    std::cout << "Finished Fefactoring" << std::endl;

    std::cout << "Forces Used:" << std::endl;
    for (auto& force : wash::program_meta->scalar_force_list) {
        std::cout << "scalar " << force << std::endl;
    }

    for (auto& force : wash::program_meta->vector_force_list) {
        std::cout << "vector " << force << std::endl;
    }

    write_particle_initialiser((std::string) "build/tmp/" + wash::files::app_str + "/particle_data.cpp",
                               wash::program_meta->scalar_force_list, wash::program_meta->vector_force_list);

    std::cout << "Written particle_data.cpp implementation. Our work here is done!" << std::endl;

    std::cout << "Not Quite! Let's also compile this program." << std::endl;

    std::string flags = "";
    for (int ii = doubledashidx + 1; ii < new_args.size(); ii++) {
        if (new_args[ii] == "-DDIM=2") {
            flags += " -DDIM=" + std::to_string(wash::program_meta->simulation_dimension);
        } else if (new_args[ii] == "-DWASH_HDF5_SUPPORT") {
            flags += " " + new_args[ii];
            flags += " -lhdf5 -L$HDF5_DIR/lib -I$HDF5_DIR/include";
        } else {
            flags += " " + new_args[ii];
        }
    }

    // TODO: Maybe a better way to call MPICXX and one with like decent command passing
    // TODO: Get a simulation name/output!
    std::string command =
        "OMPI_CXX=clang++ mpicxx -std=c++17 build/tmp/" + wash::files::app_str + "/*.cpp " + flags + " -O3 -o build/wash_result";

    int compile_result = system(command.c_str());
    if (compile_result != 0) {
        std::cerr << "Compilation Failed! :(" << std::endl;
        return 1;
    }

    std::cout << "Finished. Source app str is " << wash::files::app_str << std::endl;
    return 0;
}

void write_particle_initialiser(std::string path, std::vector<std::string> scalar_f,
                                std::vector<std::string> vector_f) {
    std::string output_str =
        "#include \"particle_data.hpp\" \n"
        "namespace wash {\n"
        "std::vector<SimulationVecT> vector_force_pos;\n"
        "std::vector<SimulationVecT> vector_force_vel;\n"
        "std::vector<SimulationVecT> vector_force_acc;\n"
        "std::vector<double> scalar_force_mass;\n"
        "std::vector<double> scalar_force_density;\n"
        "std::vector<double> scalar_force_smoothing_length;\n";

    output_str += wash::refactor::forces::getForceDeclarationSource();

    output_str += wash::refactor::variables::getVariableDefinitionSource();

    output_str +=
        "void _initialise_particle_data(size_t particlec) {\n"
        "    wash::vector_force_pos = std::vector<SimulationVecT>(particlec);\n"
        "    wash::vector_force_vel = std::vector<SimulationVecT>(particlec);\n"
        "    wash::vector_force_acc = std::vector<SimulationVecT>(particlec);\n"
        "    wash::scalar_force_mass = std::vector<double>(particlec);\n"
        "    wash::scalar_force_density = std::vector<double>(particlec);\n"
        "    wash::scalar_force_smoothing_length = std::vector<double>(particlec);\n";

    output_str += wash::refactor::forces::getForceInitialisationSource();

    output_str += " } }";

    std::ios_base::openmode mode = std::ofstream::out;
    std::ofstream outfile(path.c_str(), mode);

    outfile << output_str.c_str() << std::endl;

    outfile.close();
}