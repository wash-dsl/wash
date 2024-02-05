#pragma once

#include "../common.hpp"

namespace wash {

namespace refactor {

namespace forces {

    // === Matchers for getting the force information === 

    extern StatementMatcher AddForceVectorMatcher;
    extern StatementMatcher AddForceScalarMatcher;

    template<ForceType type>
    void HandleRegisterForces(const MatchFinder::MatchResult& Result);

    std::string getForceDeclarationSource();
    std::string getForceInitialisationSource();

    // === Refactoring for getting / setting forces === s

    extern StatementMatcher GetForceScalarMatcher;
    extern StatementMatcher GetForceVectorMatcher;

    template <ForceType type>
    class HandleGetForce : public WashMatchCallback {
    public:
        void run(const MatchFinder::MatchResult &Result);
    };
    
    extern StatementMatcher SetForceScalarMatcher;
    extern StatementMatcher SetForceVectorMatcher;

    template <ForceType type>
    class HandleSetForce : public WashMatchCallback {
    public:
        void run(const MatchFinder::MatchResult &Result);
    };

    // === Refactoring for the forces definition === 

    extern DeclarationMatcher InsertForcesDefinitionMatcher;

    class HandleInsertForcesDefinition : public WashMatchCallback {
    public:
        void run(const MatchFinder::MatchResult &Result);
    };

    // === Refactoring for the pre-defined properties === 

    extern StatementMatcher GetPosMatcher;
    extern StatementMatcher GetVelMatcher;
    extern StatementMatcher GetAccMatcher;

    extern StatementMatcher GetDensityMatcher;
    extern StatementMatcher GetMassMatcher;
    extern StatementMatcher GetSmoothingLengthMatcher;

    template <ForceType type, const char* name>
    class HandleGetProperty : public WashMatchCallback {
    public:
        void run(const MatchFinder::MatchResult &Result);
    };

    extern StatementMatcher SetPosMatcher;
    extern StatementMatcher SetVelMatcher;
    extern StatementMatcher SetAccMatcher;

    extern StatementMatcher SetDensityMatcher;
    extern StatementMatcher SetMassMatcher;
    extern StatementMatcher SetSmoothingLengthMatcher;

    template <ForceType type, const char* name>
    class HandleSetProperty : public WashMatchCallback {
    public:
        void run(const MatchFinder::MatchResult &Result);
    };

    // === TODO: Matchers for the IO when iterating through all forces === 

    // === Helper functions for defining the property matchers ===
    const StatementMatcher PropertyGetMatcher(const char* propertyName);

    const StatementMatcher PropertySetMatcher(const char* propertyName);
}

}

}