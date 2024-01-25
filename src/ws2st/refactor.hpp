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
#include "common.hpp"

namespace wash {

    class EnumRefactoring : public RefactoringTool {
        // 1. find the places where we access the forces using a string
        // 2. create replacement with the enum access
        // 3. save it out with the enum header file + the wisb sources
        // 4. pass the translated source through the clang compiler
    };

}