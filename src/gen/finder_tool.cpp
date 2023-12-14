/**
 * @file finder_tool.cpp
 * @author jamesm2w
 * @brief Runs the Find Wash Function as a standalone clang tool on passed in code.
 * @version 0.1
 * @date 2023-12-14
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "finder.hpp"

class FindWashFunctionAction : public clang::ASTFrontendAction {
public:
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &Compiler,
                                                                  llvm::StringRef InFile) {
        return std::make_unique<FindWashFunctionConsumer>(&Compiler.getASTContext());
    }
};

int main(int argc, char **argv) {
    if (argc > 1) {
        clang::tooling::runToolOnCode(std::make_unique<FindWashFunctionAction>(), argv[1]);
    } else {
        std::string buf;
        std::cin >> buf;
        clang::tooling::runToolOnCode(std::make_unique<FindWashFunctionAction>(), buf);
    }
}