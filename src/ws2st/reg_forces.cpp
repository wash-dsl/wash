/**
 * @file reg_forces.cpp
 * @author james
 * @brief defines some implementations for the static/global things used to find registered forces
 * @version 0.1
 * @date 2024-01-22
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "reg_forces.hpp"

namespace wash {
    // Matches calls to add force vector inside the main function
    StatementMatcher addForceVectorMatcher = traverse(TK_IgnoreUnlessSpelledInSource, callExpr(
        hasAncestor(functionDecl(hasName("main"))),
        hasDescendant(
            declRefExpr(
                to(functionDecl(
                    hasName("wash::add_force_vector")
                ))
            )
        ),
        hasArgument(0, ignoringImplicit( stringLiteral().bind("forceName") ))
    ).bind("callExpr"));

    // Matches calls to add force scalar inside the main function
    StatementMatcher addForceScalarMatcher = traverse(TK_IgnoreUnlessSpelledInSource, callExpr(
        hasAncestor(functionDecl(hasName("main"))),
        hasDescendant(
            declRefExpr(
                to(functionDecl(
                    hasName("wash::add_force_scalar")
                ))
            )
        ),
        hasArgument(0, ignoringImplicit( stringLiteral().bind("forceName") ))
    ).bind("callExpr"));

    // define some implementations
    std::unordered_set<std::string> RegisterForces::scalar_forces = {};
    std::unordered_set<std::string> RegisterForces::vector_forces = {};
    std::unordered_map<std::string, FullSourceLoc> RegisterForces::force_meta = {};
}