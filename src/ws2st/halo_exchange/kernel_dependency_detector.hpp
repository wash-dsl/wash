#pragma once

#include "../common.hpp"
#include "../variables/variables.hpp"

namespace ws2st {
namespace dependency_detection {
    

extern StatementMatcher AddForceKernelMatcher;
extern void RegisterForceKernel(const MatchFinder::MatchResult &Result, Replacements& Replace);

extern StatementMatcher AddInitKernelMatcher;
extern void RegisterInitKernel(const MatchFinder::MatchResult &Result, Replacements& Replace);

extern StatementMatcher SetNeighbourSearchKernelMatcher;
extern void RegisterNeighbourSearchKernel(const MatchFinder::MatchResult &Result, Replacements& Replace);

extern StatementMatcher GenericFunctionCallInFunction;
extern void HandleFunctionCallInFunction(const MatchFinder::MatchResult &Result, Replacements &Replace);

extern StatementMatcher ForceAssignmentInFunction;
extern void RegisterForceAssignment(const MatchFinder::MatchResult &Result, Replacements& Replace);

extern StatementMatcher ForceReadInFunction;
extern void RegisterForceRead(const MatchFinder::MatchResult &Result, Replacements& Replace);

enum class PropertyList { Pos, Vel, Acc, Density, Mass, SmoothingLength };
const std::string getPropertyName(PropertyList property); 

const StatementMatcher ReadPropertyMatcher(const std::string propertyName);
const StatementMatcher WritePropertyMatcher(const std::string propertyName);

template <PropertyList property>
void RegisterReadProperty(const MatchFinder::MatchResult& Result, Replacements& Replace);

template <PropertyList property>
void RegisterWriteProperty(const MatchFinder::MatchResult& Result, Replacements& Replace);

extern StatementMatcher PosReadInFunction;
extern StatementMatcher VelReadInFunction;
extern StatementMatcher AccReadInFunction;
extern StatementMatcher DensityReadInFunction;
extern StatementMatcher MassReadInFunction;
extern StatementMatcher SmoothingLengthReadInFunction;

extern WashCallbackFn RegisterPosRead;
extern WashCallbackFn RegisterVelRead;
extern WashCallbackFn RegisterAccRead;
extern WashCallbackFn RegisterDensityRead;
extern WashCallbackFn RegisterMassRead;
extern WashCallbackFn RegisterSmoothingLengthRead;

extern StatementMatcher PosWriteInFunction;
extern StatementMatcher VelWriteInFunction;
extern StatementMatcher AccWriteInFunction;
extern StatementMatcher DensityWriteInFunction;
extern StatementMatcher MassWriteInFunction;
extern StatementMatcher SmoothingLengthWriteInFunction;

extern WashCallbackFn RegisterPosWrite;
extern WashCallbackFn RegisterVelWrite;
extern WashCallbackFn RegisterAccWrite;
extern WashCallbackFn RegisterDensityWrite;
extern WashCallbackFn RegisterMassWrite;
extern WashCallbackFn RegisterSmoothingLengthWrite;

}

}