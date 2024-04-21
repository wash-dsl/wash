#pragma once

#include "../common.hpp"

namespace ws2st {

namespace refactor {

namespace forces {

    enum class PropertyList { Pos, Vel, Acc, Density, Mass, SmoothingLength, Id };
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

    std::string getForceDeclarationSourceWithCornerstone();
    std::string getForceInitialisationSourceWithCornerstone();

    // === Refactoring for getting / setting forces === s

    extern StatementMatcher GetForceScalarMatcher;
    extern StatementMatcher GetForceVectorMatcher;

    template <ForceType type>
    void HandleGetForce(const MatchFinder::MatchResult& Result, Replacements& Replace);
    
    extern WashCallbackFn HandleGetForceScalar;
    extern WashCallbackFn HandleGetForceVector;
    extern WashCallbackFn HandleGetForceVectorWithCornerstone;

    extern StatementMatcher SetForceScalarMatcher;
    extern StatementMatcher SetForceVectorMatcher;

    template <ForceType type>
    void HandleSetForce(const MatchFinder::MatchResult& Result, Replacements& Replace);

    extern WashCallbackFn HandleSetForceScalar;
    extern WashCallbackFn HandleSetForceVector;
    extern WashCallbackFn HandleSetForceVectorWithCornerstone;

    // === Refactoring for the forces definition === 

    extern DeclarationMatcher InsertForcesDefinitionMatcher;

    void HandleInsertForcesDefinition(const MatchFinder::MatchResult& Result, Replacements& Replace);
    void HandleInsertForcesDefinitionWithCornerstone(const MatchFinder::MatchResult &Result, Replacements& Replace);

    // === Refactoring for the pre-defined properties === 

    extern StatementMatcher GetPosMatcher;
    extern StatementMatcher GetVelMatcher;
    extern StatementMatcher GetAccMatcher;

    extern StatementMatcher GetDensityMatcher;
    extern StatementMatcher GetMassMatcher;
    extern StatementMatcher GetSmoothingLengthMatcher;
    extern StatementMatcher GetIdMatcher;

    template <ForceType type, PropertyList property>
    void HandleGetProperty(const MatchFinder::MatchResult& Result, Replacements& Replace);
    template<PropertyList property>
    void HandleGetPropertyWithCornerstone(const MatchFinder::MatchResult& Result, Replacements& Replace);

    extern WashCallbackFn HandleGetPos;
    extern WashCallbackFn HandleGetVel;
    extern WashCallbackFn HandleGetAcc;

    extern WashCallbackFn HandleGetPosWithCornerstone;
    extern WashCallbackFn HandleGetVelWithCornerstone;
    extern WashCallbackFn HandleGetAccWithCornerstone;

    extern WashCallbackFn HandleGetDensity;
    extern WashCallbackFn HandleGetMass;
    extern WashCallbackFn HandleGetSmoothingLength;
    extern WashCallbackFn HandleGetId;

    extern StatementMatcher SetPosMatcher;
    extern StatementMatcher SetVelMatcher;
    extern StatementMatcher SetAccMatcher;

    extern StatementMatcher SetDensityMatcher;
    extern StatementMatcher SetMassMatcher;
    extern StatementMatcher SetSmoothingLengthMatcher;

    template <ForceType type, PropertyList property>
    void HandleSetProperty(const MatchFinder::MatchResult& Result, Replacements& Replace);
    template <PropertyList property>
    void HandleSetPropertyWithCornerstone(const MatchFinder::MatchResult& Result, Replacements& Replace);

    extern WashCallbackFn HandleSetPos;
    extern WashCallbackFn HandleSetVel;
    extern WashCallbackFn HandleSetAcc;

    extern WashCallbackFn HandleSetPosWithCornerstone;
    extern WashCallbackFn HandleSetVelWithCornerstone;
    extern WashCallbackFn HandleSetAccWithCornerstone;

    extern WashCallbackFn HandleSetDensity;
    extern WashCallbackFn HandleSetMass;
    extern WashCallbackFn HandleSetSmoothingLength;

    // === Helper functions for defining the property matchers ===
    const StatementMatcher PropertyGetMatcher(const char* propertyName);
    const StatementMatcher PropertySetMatcher(const char* propertyName);
}

}

}