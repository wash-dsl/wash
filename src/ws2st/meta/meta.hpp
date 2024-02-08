#pragma once

#include "../common.hpp"

namespace wash {

namespace refactor {

namespace meta {

    extern StatementMatcher SetDimensionMatcher;

    void HandleSetDimension(const MatchFinder::MatchResult& Result, Replacements& Replace);

    extern DeclarationMatcher SimulationVecTMatcher;

    void HandleSimulationVecTMatcher(const MatchFinder::MatchResult& Result, Replacements& Replace);

    extern DeclarationMatcher DefineForceAccessFnMatcher;

    void DefineForceAccessFns(const MatchFinder::MatchResult& Result, Replacements& Replace);
}

}

}