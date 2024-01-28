#include "ws2st.hpp"

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory WashS2STCategory("Wash S2S Translator");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nThis is the WaSH source-to-source translator tool.\n");

    
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

    int new_argc = 0;
    std::vector<const char*> args_cstr = {};
    std::transform(new_args.cbegin(), new_args.cend(), std::back_inserter(args_cstr),
                   [&new_argc](const std::string& s) { new_argc++; return s.c_str(); });

    const char** new_argv = (const char**)(args_cstr.data());

    for (auto i = 0; i < new_argc; i++) {
        std::cout << new_argv[i] << " ";
    }
    std::cout << "\n";

    auto ExpectedParser = CommonOptionsParser::create(new_argc, new_argv, WashS2STCategory);

    if (!ExpectedParser) {
        // Fail gracefully for unsupported options.
        llvm::errs() << ExpectedParser.takeError();
        throw "Couldn't create parser";
    }
    CommonOptionsParser& OptionsParser = ExpectedParser.get();
    
    RefactoringTool informationTool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
    int success = wash::gatherSimulationInformation(informationTool);

    if (success != 0) {
        return success;
    }

    // output the simulation information
    std::cout << "simulation dimension" << wash::information::simulation_dimension << std::endl;

    std::cout << "scalars" << std::endl;
    for (auto x : wash::information::scalar_forces) {
        std::cout << "\t" << x << std::endl;
    }
    std::cout << "vectors" << std::endl;
    for (auto x : wash::information::vector_forces) {
        std::cout << "\t" << x << std::endl;
    }
    std::cout << "starting refactoring" << std::endl;

    // TODO: probably like better than this
    new_args.push_back("-DDIM=" + std::to_string(wash::information::simulation_dimension));

    // CommonOptionsParser& getOptions = create_tool(new_args).get();
    RefactoringTool getRefactorTool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
    int res = wash::getForceRewriting(getRefactorTool, wash::information::scalar_forces,
                                      wash::information::vector_forces, wash::information::simulation_dimension);

    if (res != 0) {
        return res;
    }

    std::cout << "finished get pass" << std::endl;

    // CommonOptionsParser& setOptions = create_tool(new_args).get();
    RefactoringTool setRefactorTool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
    res = wash::setForceRewriting(setRefactorTool);

    if (res != 0) {
        return res;
    }

    std::cout << "finished refactoring" << std::endl;

    write_particle_initialiser((std::string) "build/tmp/" + wash::files::app_str + "/particle_data.cpp",
                               wash::information::scalar_forces, wash::information::vector_forces);

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