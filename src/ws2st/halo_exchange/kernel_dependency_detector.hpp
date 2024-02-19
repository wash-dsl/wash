#pragma once

#include "../common.hpp"
#include "../variables/variables.hpp"

namespace wash {
namespace dependency_detection {
    

extern StatementMatcher AddForceKernelMatcher;
extern void RegisterForceKernel(const MatchFinder::MatchResult &Result, Replacements& Replace);

extern StatementMatcher ForceAssignmentInFunction;
extern void RegisterForceAssignment(const MatchFinder::MatchResult &Result, Replacements& Replace);

extern StatementMatcher PosAssignmentInFunction;
extern void RegisterPosAssignment(const MatchFinder::MatchResult &Result, Replacements& Replace);

extern StatementMatcher VelAssignmentInFunction;
extern void RegisterVelAssignment(const MatchFinder::MatchResult &Result, Replacements& Replace);

extern StatementMatcher AccAssignmentInFunction;
extern void RegisterAccAssignment(const MatchFinder::MatchResult &Result, Replacements& Replace);

extern StatementMatcher ForceReadInFunction;
extern void RegisterForceRead(const MatchFinder::MatchResult &Result, Replacements& Replace);

extern StatementMatcher PosReadInFunction;
extern void RegisterPosRead(const MatchFinder::MatchResult &Result, Replacements& Replace);

extern StatementMatcher VelReadInFunction;
extern void RegisterVelRead(const MatchFinder::MatchResult &Result, Replacements& Replace);

extern StatementMatcher AccReadInFunction;
extern void RegisterAccRead(const MatchFinder::MatchResult &Result, Replacements& Replace);

}}