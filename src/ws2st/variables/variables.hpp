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

namespace wash {

namespace refactor {

namespace variables {

    extern StatementMatcher RegisterVariableMatcher;

    class HandleRegisterVariable : public WashMatchCallback {
    public:
        void run(const MatchFinder::MatchResult &Result);
    };

    std::string getVariableDeclarationSource();

    extern StatementMatcher SetVariableMatcher;

    class HandleSetVariable : public WashMatchCallback {
    public:
        void run(const MatchFinder::MatchResult &Result);
    };

    extern StatementMatcher GetVariableMatcher;

    class HandleGetVariable : public WashMatchCallback {
    public:
        void run(const MatchFinder::MatchResult &Result);
    };

    extern StatementMatcher HandleVariablesMatcher;

    class HandleVariableList : public WashMatchCallback {
    public:
        void run(const MatchFinder::MatchResult &Result);
    };

}

}

}