#pragma once

#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/RefactoringCallbacks.h"
#include "clang/Tooling/Tooling.h"

#include <any>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_set>
#include <variant>
#include <memory>
#include <cstdlib> 

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

namespace wash {
    // Disambiguate a SCALAR/VECTOR implementation
    enum class ForceType { SCALAR, VECTOR };

    /**
     * @brief Defines the type of the callback function which takes a MatchResult containing AST information about the match
     * and a mutable reference to the Replacements list which can be added to by the callback function
     */
    typedef void(* WashCallbackFn)(const MatchFinder::MatchResult &, Replacements &);

    /**
     * @brief Struct to hold dependencies
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
        std::unordered_map<std::string, KernelDependencies> kernels_dependency_map;
        
    };

    // Global meta information about the simulation
    extern std::shared_ptr<WashProgramMeta> program_meta;
    
    /**
     * @brief A specialised refactoring callback type for Wash using custom function pointers
     * instead of subclassing tooling::RefactoringCallback and implemented ...::run a million times
     */
    class WashMatchCallback : public tooling::RefactoringCallback {
    private:
        WashCallbackFn callback_ptr; // Pointer to function called when there's a match
    public:
        WashMatchCallback(WashCallbackFn callback_ptr) : callback_ptr(callback_ptr), RefactoringCallback() {
            // std::cout << "1: created callback to " << reinterpret_cast<const void*>(callback_ptr) << std::endl;
        }

        virtual void run(const MatchFinder::MatchResult &Result) { 
            // std::cout << "3: running callback " << this << std::endl;
            callback_ptr(Result, this->getReplacements()); 
        }

        WashCallbackFn getCallback() {
            return callback_ptr;
        }

        friend std::ostream& operator<<(std::ostream& os, const WashMatchCallback& obj) {
            os << "WashMatchCallback[ WashCallbackFn(" << obj.callback_ptr << ")->" << reinterpret_cast<const void*>(obj.callback_ptr) << "]";
            return os;
        }
    };


    /**
     * @brief Developer defined registration of a refactoring action - a (<T>Matcher, CallbackFn) pair
     */
    class WashRefactoringAction {
    private:
        // Matchers can be one of many types -- currently just Statement/Declaration -- TODO: Add more as needed
        std::variant<StatementMatcher*, DeclarationMatcher*> matcher;
        WashCallbackFn callbackfn_ptr; // Function pointer to use
    public:
        WashRefactoringAction(StatementMatcher* matcher_ptr, WashCallbackFn callback_ptr)
            : matcher(matcher_ptr), callbackfn_ptr(callback_ptr) {
                // std::cout << "0: created stmt matcher with " << reinterpret_cast<const void*>(callbackfn_ptr) << " " << callbackfn_ptr << std::endl;
            }

        WashRefactoringAction(DeclarationMatcher* matcher_ptr, WashCallbackFn callback_ptr)
            : matcher(matcher_ptr), callbackfn_ptr(callback_ptr) {
                // std::cout << "0: created decl matcher with " << reinterpret_cast<const void*>(callbackfn_ptr) << " " << callbackfn_ptr << std::endl;
            }

        WashCallbackFn getCallbackFn() {
            return callbackfn_ptr;
        }

        std::variant<StatementMatcher*, DeclarationMatcher*> getMatcher() {
            return matcher;
        }
    };

}

/**
 * @brief Get the text of a source code range or not if it's not there
 * 
 * @param ctx 
 * @param srcRange 
 * @return std::optional<std::string> 
 */
std::optional<std::string> getSourceText(ASTContext *ctx, SourceRange srcRange);