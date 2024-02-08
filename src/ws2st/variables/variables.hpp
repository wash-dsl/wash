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

    // Matches calls to register a variable, which can optionally have a default value
    extern StatementMatcher RegisterVariableMatcher;
    extern StatementMatcher RegisterVariableNoInitMatcher;

    // Remove the call, and register the name and initialiser value in the program info. 
    void HandleRegisterVariable(const MatchFinder::MatchResult &Result, Replacements& Replace);

    // Return code to create the variable storage and then initialise their values.
    std::string getVariableDeclarationSource();
    std::string getVariableDefinitionSource();

    // Calls to set a variable value
    extern StatementMatcher SetVariableMatcher;

    void HandleSetVariable(const MatchFinder::MatchResult &Result, Replacements& Replace);

    // Calls to get a variable value
    extern StatementMatcher GetVariableMatcher;

    void HandleGetVariable(const MatchFinder::MatchResult &Result, Replacements& Replace);

    // Calls to get a reference to a variable useful when you need a pointer type explicitly at compile time
    extern StatementMatcher GetVariableRefMatcher;

    void HandleGetVariableRef(const MatchFinder::MatchResult &Result, Replacements& Replace);

    // Calls to insert the variables declaration in the headers to ensure it's available in other compile stages too.
    extern DeclarationMatcher InsertVariablesDeclarationMatcher;

    void HandleInsertVariablesDeclaration(const MatchFinder::MatchResult &Result, Replacements& Replace);
}

}

}