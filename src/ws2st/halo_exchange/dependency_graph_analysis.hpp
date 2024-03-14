#pragma once

#include "../common.hpp"
#include "../variables/variables.hpp"

namespace ws2st {
namespace dependency_detection {

extern std::vector<bool> compute_domain_syncs();

extern std::vector<std::vector<std::string>> compute_halo_exchanges();

/*
extern DeclarationMatcher InsertDomainSyncsMatcher;
extern void HandleDomainSync(const MatchFinder::MatchResult &Result, Replacements& Replace);

extern DeclarationMatcher InsertHaloExchangeMatcher;
extern void HandleHaloExchange(const MatchFinder::MatchResult &Result, Replacements& Replace);
*/

extern DeclarationMatcher LoopRewriteMatcher;
extern void UnrollKernelDependencyLoop(const MatchFinder::MatchResult &Result, Replacements& Replace);

}}