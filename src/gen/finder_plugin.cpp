/**
 * @file finder_plugin.cpp
 * @author jamesm2w
 * @brief Implements the Find Wash Function as a plugin for clang compiler
 * @version 0.1
 * @date 2023-12-14
 *
 * @copyright Copyright (c) 2023
 */
#include "finder.hpp"

/**
 * @brief Implements the plugin through using the already defined behaviour of the frontent action.
 */
class FindWashFunctionsPlugin : public clang::PluginASTAction {
public:
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &Compiler,
                                                          llvm::StringRef InFile) override {
        return std::make_unique<FindWashFunctionConsumer>(&Compiler.getASTContext());
    }

    /**
     * @brief Allows the plugin to take in a set of arguments
     */
    bool ParseArgs(const clang::CompilerInstance &CI, const std::vector<std::string> &args) override {
        for (int i = 0; i < args.size(); i++) {
            std::cerr << "Find Wash Functions arg " << i << " = " << args[i] << std::endl;

            clang::DiagnosticsEngine &D = CI.getDiagnostics();
            if (args[i] == "test-error") {
                int DiagID = D.getCustomDiagID(clang::DiagnosticsEngine::Error, "Invalid Argument \"%0\"");
                D.Report(DiagID) << args[i];
                return false;
            }
        }

        if (!args.empty() && args[0] == "help") {
            std::cerr << "Find Wash Functions: Usage: None." << std::endl;
        }

        return true;
    }

    /**
     * @brief Sets when this plugin should be run. Before Main Action = Before Code Gen I believe
     */
    clang::PluginASTAction::ActionType getActionType() override { return AddBeforeMainAction; }
};

// Register the plugin on the registry
static clang::FrontendPluginRegistry::Add<FindWashFunctionsPlugin> X("FindWashFns", "find wash functions");
