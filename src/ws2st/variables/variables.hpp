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

    void HandleRegisterVariable(const MatchFinder::MatchResult &Result, Replacements& Replace);

    std::string getVariableDeclarationSource();

    extern StatementMatcher SetVariableMatcher;

    void HandleSetVariable(const MatchFinder::MatchResult &Result, Replacements& Replace);

    extern StatementMatcher GetVariableMatcher;

    void HandleGetVariable(const MatchFinder::MatchResult &Result, Replacements& Replace);

    extern StatementMatcher HandleVariablesMatcher;

    void HandleVariableList(const MatchFinder::MatchResult &Result, Replacements& Replace);

}

}

}