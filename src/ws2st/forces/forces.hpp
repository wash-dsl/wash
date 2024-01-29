#pragma once

#include "../common.hpp" 
#include "../ws2st.hpp"

namespace wash {

namespace refactor {

namespace forces {

    // === Matchers for getting the force information === 

    extern StatementMatcher AddForceVectorMatcher;
    extern StatementMatcher AddForceScalarMatcher;
    extern std::vector<std::string> scalar_force_list;
    extern std::vector<std::string> vector_force_list;
    extern std::unordered_map<std::string, FullSourceLoc> force_meta;

    template<ForceType type>
    class HandleRegisterForces : public tooling::RefactoringCallback {
    public:
        void run(const MatchFinder::MatchResult &Result);
    };

    std::string getForceDeclarationSource();

    // === Refactoring for getting / setting forces === 

    extern StatementMatcher GetForceScalarMatcher;
    extern StatementMatcher GetForceVectorMatcher;

    template <ForceType type>
    class HandleGetForce : public tooling::RefactoringCallback {
    public:
        void run(const MatchFinder::MatchResult &Result);
    };
    
    extern StatementMatcher SetForceScalarMatcher;
    extern StatementMatcher SetForceVectorMatcher;

    template <ForceType type>
    class HandleSetForce : public tooling::RefactoringCallback {
    public:
        void run(const MatchFinder::MatchResult &Result);
    };

    // === Refactoring for the forces definition === 

    extern DeclarationMatcher InsertForcesDefinitionMatcher;

    class HandleInsertForcesDefinition : public tooling::RefactoringCallback {
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

    template <ForceType type>
    class HandleGetProperty : public tooling::RefactoringCallback {
    private:
        std::string name;
    public:
        HandleGetProperty(std::string name) : name(name) {}
        void run(const MatchFinder::MatchResult &Result);
    };

    extern StatementMatcher SetPosMatcher;
    extern StatementMatcher SetVelMatcher;
    extern StatementMatcher SetAccMatcher;

    extern StatementMatcher SetDensityMatcher;
    extern StatementMatcher SetMassMatcher;
    extern StatementMatcher SetSmoothingLengthMatcher;

    template <ForceType type>
    class HandleSetProperty : public tooling::RefactoringCallback {
    private:
        std::string name;
    public:
        HandleSetProperty(std::string name) : name(name) {}
        void run(const MatchFinder::MatchResult &Result);
    };

    // === Helper functions for defining the property matchers ===
    const StatementMatcher PropertyGetMatcher(const char* propertyName);

    const StatementMatcher PropertySetMatcher(const char* propertyName);
}

}

}