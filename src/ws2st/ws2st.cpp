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
    { Implementations::wser, { 1, 0, 0, 0x40 | 2, "wser", "./src/impl/wser" } },
    { Implementations::wisb, { 1, 0, 0, 0x40 | 2, "wisb", "./src/impl/wisb" } },
    { Implementations::west, { 1, 0, 0, 0x40 | 2, "west", "./src/impl/west" } },
    { Implementations::cstone, { 1, 1, 0, 0x80 | 3, "cstone", "./src/impl/cstone" } },
    { Implementations::wone, { 1, 1, 0, 0x80 | 3, "wone", "./src/impl/wone" } }
};

std::vector<std::string> UserFiles = {}; // User source files copied in
std::vector<std::string> BackendFiles = {}; // Backend files `src/impl/xyza`
std::vector<std::string> PublicHeaders = {}; // `include/` public API headers
std::vector<std::string> AllFiles = {}; // All files in the temp directory
std::vector<std::string> NoFiles = {}; // Jut doesn't run the pass (Why?)

#ifndef TEST_MAIN
int main(int argc, const char** argv) {

    WashOptions programOptions = ws2st::args::parseCommandLine(argc, argv);
    std::vector<std::string> allSources = ws2st::files::assembleSourceWithBackend(programOptions);
    AllFiles = allSources;

    ws2st::refactor::runRefactoring(programOptions);

    ws2st::compile::compileWashApplication(programOptions);

    return 0;
}
#endif