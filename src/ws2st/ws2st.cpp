#include "ws2st.hpp"

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nThis is the WaSH source-to-source translator tool.\n");

/// TODO: Move this implementation out of the main file as well
std::optional<std::string> getSourceText(ASTContext* ctx, SourceRange srcRange) {
    const auto& SM = ctx->getSourceManager();
    const auto langOpts = ctx->getLangOpts();
    if (srcRange.isValid()) {
        auto code = Lexer::getSourceText(CharSourceRange::getTokenRange(srcRange), SM, langOpts);
        return code.str();
    }

    return std::nullopt;
}

Implementations default_impl = Implementations::wone;

std::unordered_map<Implementations, ImplementationFeatures> api_impls = {
    { Implementations::wser, { 1, 0, 0, 0x40 & 2, "wser", "./src/impl/wser" } },
    { Implementations::wisb, { 1, 0, 0, 0x40 & 2, "wisb", "./src/impl/wisb" } },
    { Implementations::west, { 1, 0, 0, 0x40 & 2, "west", "./src/impl/west" } },
    { Implementations::cstone, { 1, 1, 0, 0x80 & 3, "cstone", "./src/impl/cstone" } },
    { Implementations::wone, { 1, 1, 0, 0x80 & 3, "wone", "./src/impl/wone" } }
};

std::vector<std::string> UserFiles = {}; // User source files copied in
std::vector<std::string> BackendFiles = {}; // Backend files `src/impl/xyza`
std::vector<std::string> PublicHeaders = {}; // `include/` public API headers
std::vector<std::string> AllFiles = {}; // All files in the temp directory
std::vector<std::string> NoFiles = {}; // Jut doesn't run the pass (Why?)

int main(int argc, const char** argv) {

    WashOptions programOptions = ws2st::args::parseCommandLine(argc, argv);
    std::vector<std::string> allSources = ws2st::files::assembleSourceWithBackend(programOptions);
    AllFiles = allSources;

    ws2st::refactor::runRefactoring(programOptions);

    ws2st::compile::compileWashApplication(programOptions);

    /// TODO: Run the final file writing and then compilation step here as well. 

    // std::vector<std::filesystem::path> files = wash::files::copy_wash_source(argv[1]);
    // std::vector<std::string> new_args;

    // new_args.push_back((std::string)argv[0]);

    // for (const auto& path : files) {
    //     new_args.push_back(path);
    // }

    // for (int i = 2; i < argc; i++) {
    //     new_args.push_back((std::string)argv[i]);
    // }

    // size_t doubledashidx = -1;
    // for (int ii = 0; ii < new_args.size(); ii++) {
    //     if (new_args[ii] == "--") {
    //         doubledashidx = ii;
    //     }
    // }

    // if (doubledashidx == -1) {
    //     new_args.push_back("--");
    //     doubledashidx = new_args.size() - 1;
    // }

    // new_args.push_back("-I/usr/lib64/clang/16/include");
    // new_args.push_back("-std=c++17");
    // new_args.push_back("-DWASH_HDF5");
    // new_args.push_back("-DWASH_WEST");
    // new_args.push_back("-DDIM=3");
    // new_args.push_back("-fopenmp");

    /// TODO: Optionally enable MPI via some 
    /// command line arugmnts 
    /// `mpicxx -showme`

    // wash::refactor::runRefactoringStages(new_args);

    // std::cout << "Finished Fefactoring" << std::endl;

    // std::cout << "Forces Used:" << std::endl;
    // for (auto& force : wash::program_meta->scalar_force_list) {
    //     std::cout << "scalar " << force << std::endl;
    // }

    // for (auto& force : wash::program_meta->vector_force_list) {
    //     std::cout << "vector " << force << std::endl;
    // }

    // write_particle_initialiser((std::string) "build/tmp/" + wash::files::app_str + "/particle_data.cpp",
    //                            wash::program_meta->scalar_force_list, wash::program_meta->vector_force_list);

    // std::cout << "Written particle_data.cpp implementation. Our work here is done!" << std::endl;

    // std::cout << "Not Quite! Let's also compile this program." << std::endl;

    // std::string flags = "";
    // for (int ii = doubledashidx + 1; ii < new_args.size(); ii++) {
    //     if (new_args[ii] == "-DDIM=2") {
    //         flags += " -DDIM=" + std::to_string(wash::program_meta->simulation_dimension);
    //     } else if (new_args[ii] == "-DWASH_HDF5") {
    //         flags += " " + new_args[ii];
    //         std::string hdf5_dir = getenv("HDF5_DIR");
    //         std::cout << "Picked up hdf5 dir " << hdf5_dir << std::endl;
    //         flags += " -lhdf5 -L" + hdf5_dir + "/lib -I" + hdf5_dir + "/include";
    //     } else {
    //         flags += " " + new_args[ii];
    //     }
    // }

    // // TODO: Maybe a better way to call MPICXX and one with like decent command passing
    // // TODO: Get a simulation name/output!
    // std::string command =
    //     "OMPI_CXX=clang++ mpicxx -std=c++17 build/tmp/" + wash::files::app_str + "/*.cpp " + flags + " -O3 -o build/wash_result";
    // std::cout << command << std::endl;

    // int compile_result = system(command.c_str());
    // if (compile_result != 0) {
    //     std::cerr << "Compilation Failed! :( Code " << compile_result << std::endl;
    //     return 1;
    // }

    // std::cout << "Finished. Source app str is " << wash::files::app_str << std::endl;
    // return 0;
}