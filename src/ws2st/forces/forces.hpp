#pragma once

#include "../common.hpp"

namespace wash {

namespace refactor {

namespace forces {

    enum class PropertyList { Pos, Vel, Acc, Density, Mass, SmoothingLength };
    const std::string propertyName(PropertyList property); 

    // === Matchers for getting the force information === 

    extern StatementMatcher AddForceVectorMatcher;
    extern StatementMatcher AddForceScalarMatcher;

    template<ForceType type>
    void HandleRegisterForces(const MatchFinder::MatchResult& Result, Replacements& Replace);

    extern WashCallbackFn HandleRegisterForcesScalar;
    extern WashCallbackFn HandleRegisterForcesVector;

    std::string getForceDeclarationSource();
    std::string getForceInitialisationSource();

    // === Refactoring for getting / setting forces === s

    extern StatementMatcher GetForceScalarMatcher;
    extern StatementMatcher GetForceVectorMatcher;

    template <ForceType type>
    void HandleGetForce(const MatchFinder::MatchResult& Result, Replacements& Replace);
    
    extern WashCallbackFn HandleGetForceScalar;
    extern WashCallbackFn HandleGetForceVector;

    extern StatementMatcher SetForceScalarMatcher;
    extern StatementMatcher SetForceVectorMatcher;

    template <ForceType type>
    void HandleSetForce(const MatchFinder::MatchResult& Result, Replacements& Replace);

    extern WashCallbackFn HandleSetForceScalar;
    extern WashCallbackFn HandleSetForceVector;

    // === Refactoring for the forces definition === 

    extern DeclarationMatcher InsertForcesDefinitionMatcher;

    // class HandleInsertForcesDefinition : public WashMatchCallback {
    // public:
    //     void run(const MatchFinder::MatchResult &Result);
    // };
    void HandleInsertForcesDefinition(const MatchFinder::MatchResult& Result, Replacements& Replace);

    // === Refactoring for the pre-defined properties === 

    extern StatementMatcher GetPosMatcher;
    extern StatementMatcher GetVelMatcher;
    extern StatementMatcher GetAccMatcher;

    extern StatementMatcher GetDensityMatcher;
    extern StatementMatcher GetMassMatcher;
    extern StatementMatcher GetSmoothingLengthMatcher;

    template <ForceType type, PropertyList property>
    void HandleGetProperty(const MatchFinder::MatchResult& Result, Replacements& Replace);

    extern WashCallbackFn HandleGetPos;
    extern WashCallbackFn HandleGetVel;
    extern WashCallbackFn HandleGetAcc;

    extern WashCallbackFn HandleGetDensity;
    extern WashCallbackFn HandleGetMass;
    extern WashCallbackFn HandleGetSmoothingLength;

    extern StatementMatcher SetPosMatcher;
    extern StatementMatcher SetVelMatcher;
    extern StatementMatcher SetAccMatcher;

    extern StatementMatcher SetDensityMatcher;
    extern StatementMatcher SetMassMatcher;
    extern StatementMatcher SetSmoothingLengthMatcher;

    template <ForceType type, PropertyList property>
    void HandleSetProperty(const MatchFinder::MatchResult& Result, Replacements& Replace);

    extern WashCallbackFn HandleSetPos;
    extern WashCallbackFn HandleSetVel;
    extern WashCallbackFn HandleSetAcc;

    extern WashCallbackFn HandleSetDensity;
    extern WashCallbackFn HandleSetMass;
    extern WashCallbackFn HandleSetSmoothingLength;

    // === TODO: Matchers for the IO when iterating through all forces === 
    // extern const StatementMatcher IOForcesLoopMatcher;

    // void HandleIOForcesLoop(const MatchFinder::MatchResult& Result, Replacements& Replace);

    // === Helper functions for defining the property matchers ===
    const StatementMatcher PropertyGetMatcher(const char* propertyName);

    const StatementMatcher PropertySetMatcher(const char* propertyName);
}

}

}