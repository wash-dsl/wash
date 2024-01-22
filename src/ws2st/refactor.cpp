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
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, WashS2STCategory);
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

    return 0;
}
