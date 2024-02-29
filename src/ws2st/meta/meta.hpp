#pragma once

#include "../common.hpp"

namespace ws2st {

namespace refactor {

namespace meta {

    // Matcher for calls to set the simulation dimension
    extern StatementMatcher SetDimensionMatcher;

    // Removes the set dimension call and updates the program_meta store with the value
    void HandleSetDimension(const MatchFinder::MatchResult& Result, Replacements& Replace);

    // Matches the typedef declaration of SimulationVecT
    extern DeclarationMatcher SimulationVecTMatcher;

    // Overwrites the DIM type parameter to be the simulation dimension
    void HandleSimulationVecTMatcher(const MatchFinder::MatchResult& Result, Replacements& Replace);

    // Finds the dummy struct in io.cpp which is a placeholder for the definition of several functions
    // to access the force data and string names at runtimef for the IO code.
    extern DeclarationMatcher DefineForceAccessFnMatcher;

    // Defines these force functions for scalar/vector forces and variables. Both data and names.
    void DefineForceAccessFns(const MatchFinder::MatchResult& Result, Replacements& Replace);
}

}

}