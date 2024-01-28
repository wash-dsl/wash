/**
 * @file reg_forces.hpp
 * @author james
 * @brief Defines some a basic matching to find force regsitrations and put them in some sets
 * @version 0.1
 * @date 2024-01-22
 * 
 * @copyright Copyright (c) 2024
 */
#pragma once

#include "common.hpp"
#include "variables/variables.hpp"

namespace wash {

    extern StatementMatcher addForceVectorMatcher;
    extern StatementMatcher addForceScalarMatcher;
    extern StatementMatcher setSimulationDimensionMatcher;

namespace information {

    extern std::unordered_set<std::string> scalar_forces;
    extern std::unordered_set<std::string> vector_forces;
    extern uint64_t simulation_dimension; 

    extern std::unordered_map<std::string, FullSourceLoc> force_meta;

    template<ForceType type>
    class RegisterForcesCallback : public tooling::RefactoringCallback {
    public:
        void run(const MatchFinder::MatchResult &Result);
    };

    class SimulationDimensionRefactor : public tooling::RefactoringCallback {
    public: 
        void run(const MatchFinder::MatchResult &Result);
    };
}

    int gatherSimulationInformation(RefactoringTool& Tool);

}