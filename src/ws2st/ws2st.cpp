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

int main(int argc, const char **argv) {

    // argv == wash/old --> copy over the src and original wash backend no changes made
    //      == wash/isb --> copy over the src and new wisb backend and do the enum changes ets
    //      == openmp   --> add in linking with OpenMP

    if (argc < 2) {
        std::cerr << "Usagae: " << argv[0] << " /path/to/source -- [other compiler options]" << std::endl;
        return 1;
    }

    std::vector<std::filesystem::path> files = wash::files::copy_wash_source(argv[1]);
    std::vector<char *> new_args;

    char* cstr = new char[std::strlen(argv[0]) + 1];
    std::strcpy(cstr, argv[0]);
    new_args.push_back(cstr);

    for (const auto& path : files) {
        char* cstr = new char[std::strlen(path.c_str()) + 1];
        std::strcpy(cstr, path.c_str());
        new_args.push_back(cstr);
    }

    for (int i = 2; i < argc; i++) {
        char* cstr = new char[std::strlen(argv[i]) + 1];
        std::strcpy(cstr, argv[i]);
        new_args.push_back(cstr);
    }

    std::cout << "clang tool args " << std::endl;
    for (const auto& arg : new_args) {
        std::cout << "\t" << arg << std::endl;
    }

    int new_size = static_cast<int>(new_args.size());
    const char** c_new_args = (const char **)(new_args.data());  

    auto ExpectedParser = CommonOptionsParser::create(new_size, c_new_args, WashS2STCategory);
    if (!ExpectedParser) {
        // Fail gracefully for unsupported options.
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }

    CommonOptionsParser& OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(),
    OptionsParser.getSourcePathList());

    // run through and find the registered forces
    int success = wash::RegisterForces::checkRegisteredForces(Tool);

    // todo: any error handling would be amazing
    if (success != 0) {
        return success;
    }

    // output the registered forces
    std::cout << "scalars" << std::endl;
    for (auto x : wash::RegisterForces::scalar_forces) {
        std::cout << "\t" << x << std::endl;
    }

    std::cout << "vectors" << std::endl;
    for (auto x : wash::RegisterForces::vector_forces) {
        std::cout << "\t" << x << std::endl;
    }
    std::cout << "starting refactoring" << std::endl;

//     int getForceRewriting(RefactoringTool& Tool);
// int setForceRewriting(RefactoringTool& Tool);
// int forceDeclRewriting(RefactoringTool& Tool,

    RefactoringTool getRefactorTool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
    // int res = wash::getForceRewriting(getRefactorTool);
    int res = wash::getForceRewriting(getRefactorTool, wash::RegisterForces::scalar_forces, wash::RegisterForces::vector_forces) && res;
    
    if (res != 0) {
        return res;
    }

    std::cout << "finished get pass" << std::endl;

    RefactoringTool setRefactorTool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
    res = wash::setForceRewriting(setRefactorTool);
    // int res = wash::forceNameRewriting(refactorTool, wash::RegisterForces::scalar_forces, wash::RegisterForces::vector_forces);

    if (res != 0) {
        return res;
    }

    std::cout << "finished refactoring" << std::endl;

    write_particle_initialiser((std::string) "build/tmp/" + wash::files::app_str + "/particle_data.cpp", wash::RegisterForces::scalar_forces, wash::RegisterForces::vector_forces);

    for (auto &cstr : new_args) {
        delete[] cstr;
    }

    return 0;
}
