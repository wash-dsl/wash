/**
 * @file refactor.hpp
 * @author james
 * @brief Idea is to write out the results of the refactoring using the classes defined here
 * @version 0.1
 * @date 2024-01-22
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include "arguments.hpp"
#include "common.hpp"

// #include "forces/forces.hpp"
// #include "halo_exchange/kernel_dependency_detector.hpp"
// #include "meta/meta.hpp"
// #include "variables/variables.hpp"

namespace ws2st {

    namespace refactor {

        /**
         * @brief A specialised refactoring callback type for Wash using custom function pointers
         * instead of subclassing tooling::RefactoringCallback and implementing ...::run a million times
         */
        class WashMatchCallback : public tooling::RefactoringCallback {
        private:
            WashCallbackFn callback_ptr;  // Pointer to function called when there's a match
        public:
            WashMatchCallback(WashCallbackFn callback_ptr) : callback_ptr(callback_ptr), RefactoringCallback() {}

            virtual void run(const MatchFinder::MatchResult& Result) { callback_ptr(Result, this->getReplacements()); }

            WashCallbackFn getCallback() { return callback_ptr; }

            friend std::ostream& operator<<(std::ostream& os, const WashMatchCallback& obj) {
                os << "WashMatchCallback[ WashCallbackFn(" << obj.callback_ptr << ")->"
                   << reinterpret_cast<const void*>(obj.callback_ptr) << "]";
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
            WashCallbackFn callbackfn_ptr;  // Function pointer to use
        public:
            WashRefactoringAction(StatementMatcher* matcher_ptr, WashCallbackFn callback_ptr)
                : matcher(matcher_ptr), callbackfn_ptr(callback_ptr) {}

            WashRefactoringAction(DeclarationMatcher* matcher_ptr, WashCallbackFn callback_ptr)
                : matcher(matcher_ptr), callbackfn_ptr(callback_ptr) {}

            WashCallbackFn getCallbackFn() { return callbackfn_ptr; }

            std::variant<StatementMatcher*, DeclarationMatcher*> getMatcher() { return matcher; }
        };

        /**
         * @brief Define a function to be run at the end of the refactor pass. Computation Actions will be run in the
         * order they're defined, but will be run after all refactoring actions in the pass have completed.
         */
        class WashComputationAction {
            WashComputeFn computefn_ptr;

        public:
            WashComputationAction(WashComputeFn computefn_ptr) : computefn_ptr(computefn_ptr) {}

            WashComputeFn getComputeFn() { return computefn_ptr; }
        };

        /**
         * @brief A pass of a refactoring tool over the source files running a series of actions defined
         */
        class RefactorPass {
        private:
            const std::vector<std::string>* applicable_subset;
            std::vector<WashRefactoringAction> ref_act;
            std::vector<WashComputationAction> com_act;

        public:
            RefactorPass(std::initializer_list<
                         std::variant<std::vector<std::string>*, WashRefactoringAction, WashComputationAction>>
                             actions)
                : applicable_subset(std::holds_alternative<std::vector<std::string>*>(*actions.begin())
                                        ? std::get<std::vector<std::string>*>(*actions.begin())
                                        : &AllFiles) {
                for (auto i = actions.begin() + 1; i != actions.end(); i++) {
                    if (std::holds_alternative<WashRefactoringAction>(*i)) {
                        ref_act.push_back(std::get<WashRefactoringAction>(*i));
                    } else if (std::holds_alternative<WashComputationAction>(*i)) {
                        com_act.push_back(std::get<WashComputationAction>(*i));
                    }
                }
            }

            const std::vector<WashRefactoringAction>& actions() const { return ref_act; }

            const std::vector<WashComputationAction>& computations() const { return com_act; }

            const std::vector<std::string>* files() const { return applicable_subset; }
        };

        /**
         * @brief We configure the whole tool by a series of refactoring passes
         * - each pass a stage where the tool runs through the source once
         */
        class RefactoringToolConfiguration {
        private:
            std::vector<RefactorPass> refactoring_stages;

        public:
            RefactoringToolConfiguration(std::initializer_list<RefactorPass> passes)
                : refactoring_stages(passes.begin(), passes.end()) {}

            bool run(const WashOptions& opts) const;
        };

        /**
         * @brief Runs the wash refactoring application
         * @param opts Application options
         */
        void runRefactoring(const WashOptions& opts);

        namespace config {

            /**
             * @brief Returns the set of passes specified by the implementation given
             * @param impl Implementation to use passes of
             * @returns a Configuration, which is a collection of passes
             */
            const RefactoringToolConfiguration& getConfigurationForImplementation(Implementations impl);

        }
    }

}