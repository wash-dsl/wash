#include "../common.hpp"

namespace ws2st {
    
    namespace refactor {

        namespace cornerstone {

            extern DeclarationMatcher DataSetupDecl;
            void HandleSetupDataWithCornerstone(const MatchFinder::MatchResult& Result, Replacements& Replace);

            extern DeclarationMatcher ParticleRecalculateNeighbours;
            void HandleRecalculateNeighboursWithCornerstone(const MatchFinder::MatchResult& Result, Replacements& Replace);

            extern DeclarationMatcher SyncDomainCall;
            void HandleSyncDomainWithCornerstone(const MatchFinder::MatchResult& Result, Replacements& Replace);
        
            extern DeclarationMatcher ExchangeAllHalos;
            void HandleExchangeAllHalosWithCornerstone(const MatchFinder::MatchResult& Result, Replacements& Replace);

            extern DeclarationMatcher ForceKernelExec;
            void HandleForceKernelExec(const MatchFinder::MatchResult& Result, Replacements& Replace);

            extern DeclarationMatcher ParticleClassMatcher;
            void HandleRemoveParticleClass(const MatchFinder::MatchResult& Result, Replacements& Replace);

            extern StatementMatcher ParticleCallRecalcNeighbours;
            void HandleCallRecalculateNeighbours(const MatchFinder::MatchResult& Result, Replacements& Replace);

            void DeleteParticleCpp();
        }

    }

}