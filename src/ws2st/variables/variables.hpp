/**
 * @file variables.hpp
 * @author james
 * @brief Handles refactoring for the variable API calls
 * @version 0.1
 * @date 2024-01-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once

#include "../common.hpp" 
#include "../ws2st.hpp"

namespace wash {

namespace refactor {

namespace variables {

    extern StatementMatcher RegisterVariableMatcher;
    extern std::vector<std::pair<std::string, std::string>> variable_list;

    class HandleRegisterVariable : public tooling::RefactoringCallback {
    public:
        void run(const MatchFinder::MatchResult &Result);
    };

    std::string getVariableDeclarationSource();

    extern StatementMatcher SetVariableMatcher;

    class HandleSetVariable : public tooling::RefactoringCallback {
    public:
        void run(const MatchFinder::MatchResult &Result);
    };

    extern StatementMatcher GetVariableMatcher;

    class HandleGetVariable : public tooling::RefactoringCallback {
    public:
        void run(const MatchFinder::MatchResult &Result);
    };

    extern StatementMatcher HandleVariablesMatcher;

    class HandleVariableList : public tooling::RefactoringCallback {
    public:
        void run(const MatchFinder::MatchResult &Result);
    };

}

}

}