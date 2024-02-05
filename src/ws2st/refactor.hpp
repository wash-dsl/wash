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

#include "common.hpp"

#include "forces/forces.hpp"
#include "variables/variables.hpp"

namespace wash {

namespace refactor {

    /**
     * @brief A pass of a refactoring tool over the source files running a series of actions defined
     */
    using RefactorPass = std::vector<WashRefactoringAction>;

    /**
     * @brief The whole tool is a series of refactoring passes each a stage
     */
    extern std::vector<RefactorPass> refactoring_stages;

    /**
     * @brief Runs the wash refactoring application
     */
    void runRefactoringStages(std::vector<std::string> argv);
}

}