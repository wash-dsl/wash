#pragma once

#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/RefactoringCallbacks.h"
#include "clang/Tooling/Tooling.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "llvm/Support/CommandLine.h"

#include <any>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <variant>
#include <memory>
#include <cstdlib> 
#include <vector>
#include <initializer_list>
#include <algorithm>
#include <filesystem>
#include <random>
#include <optional>

using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

enum class Implementations { wser, wisb, west, cstone, wone };

/**
 * @brief Options about how the Wash analysis will behave 
 * 
 * Whether to enable certain features via flags in the compiler analysis and then
 * again in the final compilation stages. 
 * 
 * TODO: If multiple options are enabled which are contradictory, then we should generate
 * multiple output binaries at the end.
 */
struct WashOptions {
    Implementations impl; // Which API implementation to use Default: WONE (OMP+MPI)
    bool openmp; // Whether to enable OpenMP on this output (-DWASH_OMP_SUPPORT/-fopenmp) Default: Yes
    bool mpi; // Whether to enable MPI on this output (-DWASH_MPI_SUPPORT/-lmpi) Default: Determined by mpicxx cmd
    bool hdf5; // Whether to enable HDF5 on this output (-DWASH_HDF5_SUPPORT/-lHDF5) Default: Determiend by env var
    bool debug; // Whether to use debug flags on this output. Default: false
    uint8_t dim; // The number of dimensions to use (can be overriden by source code)
    std::string input_path; // Path to the input souce directory
    std::string output_name; // Path + Name of output binary (may be appended if multiple created)
    std::string temp_path; // Temporary path of files used in compilation
    std::vector<std::string> args; // List of command line arguments to use in compilation
};

struct ImplementationFeatures {
    uint8_t openmp; // 0 - Not Supported; 1 - Can be compiled with/optionally run with; 2 - Required
    uint8_t mpi; // 0 - Not Supported; 1 - Can be compiled with/optionally run with; 2 - Required
    uint8_t cuda; // 0 - Not Supported; 1 - Can be compiled with/optionally run with; 2 - Required
    uint8_t dim; // 0x80 & (dim) = Dim required; 0x40 & (dim) = Defuault dim (others supported)

    std::string name;
    std::string source_dir;
};

extern Implementations default_impl;
extern std::unordered_map<Implementations, ImplementationFeatures> api_impls;
extern std::vector<std::string> UserFiles; // User source files copied in
extern std::vector<std::string> BackendFiles; // Backend files `src/impl/xyza`
extern std::vector<std::string> PublicHeaders; // `include/` public API headers
extern std::vector<std::string> AllFiles; // All files in the temp directory
extern std::vector<std::string> NoFiles; // Jut doesn't run the pass (Why?)

namespace ws2st {
    // Disambiguate a SCALAR/VECTOR implementation
    enum class ForceType { SCALAR, VECTOR };

    /**
     * @brief Defines the type of the callback function which takes a MatchResult containing AST information about the match
     * and a mutable reference to the Replacements list which can be added to by the callback function
     */
    typedef void(* WashCallbackFn)(const MatchFinder::MatchResult &, Replacements &);
    typedef void(* WashComputeFn)(const WashOptions& opts);

    /**
     * @brief Struct to hold two vectors of dependencies (appropriately named)
    */
    struct KernelDependencies {
        std::vector<std::string> reads_from;
        std::vector<std::string> writes_to;
    };

    /**
     * @brief Meta information about the simulation which is defined globally and can be read/written to
     * by the callback functions
     */
    struct WashProgramMeta {
        std::vector<std::string> scalar_force_list;
        std::vector<std::string> vector_force_list;
        std::unordered_map<std::string, FullSourceLoc> force_meta;

        std::vector<std::pair<std::string, std::string>> variable_list;
        int simulation_dimension;

        std::vector<std::string> kernels_list;
        std::vector<std::string> init_kernels_list;
        std::unordered_map<std::string, std::unique_ptr<KernelDependencies>> kernels_dependency_map;
        
    };

    // Global meta information about the simulation
    extern std::shared_ptr<WashProgramMeta> program_meta;
}

/**
 * @brief Get the text of a source code range or not if it's not there
 * 
 * @param ctx 
 * @param srcRange 
 * @return std::optional<std::string> 
 */
std::optional<std::string> getSourceText(ASTContext *ctx, SourceRange srcRange);