#include "kernel_dependency_detector.hpp"

namespace wash {
namespace dependency_detection {

// KERNELS

StatementMatcher AddForceKernelMatcher = traverse(TK_IgnoreUnlessSpelledInSource, callExpr(
        hasAncestor(functionDecl(hasName("main"))),
        hasDescendant(
            declRefExpr(
                to(functionDecl(anyOf(
                    hasName("wash::add_force_kernel"),
                    hasName("wash::add_update_kernel"),
                    hasName("wash::add_reduction_kernel"),
                    hasName("wash::add_void_kernel")
                )))
            )
        ),
        
        hasArgument(0, ignoringImplicit(unaryOperator(
            hasOperatorName("&"),
            hasDescendant(
                declRefExpr().bind("kernel")
            )
        ).bind("kernelPtr") ))
    ).bind("callExpr"));

void RegisterForceKernel(const MatchFinder::MatchResult &Result, Replacements& Replace) {
    const clang::CallExpr *callExpr = Result.Nodes.getNodeAs<clang::CallExpr>("callExpr");
    const clang::DeclRefExpr *kernel = Result.Nodes.getNodeAs<clang::DeclRefExpr>("kernel");

    if (!callExpr || !kernel) {
        std::cerr << "Match found without callExpr or kernelPtr" << std::endl;
        throw std::runtime_error("Kernel registration match had no kernel pointer or call node");
    }

    // Get the name of the kernel and register it in our vector
    const std::string name = kernel->getNameInfo().getAsString();

    std::cout << "  Registered kernel" << name << "\n";

    program_meta->kernels_list.push_back(name);
    
}


// ASSIGNMENTS

StatementMatcher ForceAssignmentInFunction = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl().bind("caller")),

        anyOf(
            callee(cxxMethodDecl(hasName("set_force_vector"))),
            callee(cxxMethodDecl(hasName("set_force_scalar")))
        ),

        hasArgument(0, ignoringImplicit( stringLiteral().bind("assignVariableName") ))
    ).bind("assignExpr"));

void RegisterForceAssignment(const MatchFinder::MatchResult &Result, Replacements& Replace) {
    const clang::CXXMemberCallExpr *assignExpr = Result.Nodes.getNodeAs<clang::CXXMemberCallExpr>("assignExpr");
    const clang::FunctionDecl *functionDecl = Result.Nodes.getNodeAs<clang::FunctionDecl>("caller");
    const clang::StringLiteral *assignVarName = Result.Nodes.getNodeAs<clang::StringLiteral>("assignVariableName");

    //assignExpr->dump();

    if (!assignExpr || !functionDecl || ! assignVarName) {
        std::cerr << "Match found without assignExpr, functionDecl or kernelPtr" << std::endl;
        throw std::runtime_error("Force assignment match had missing binds");
    }

    const std::string function_name = functionDecl->getNameInfo().getAsString();
    const std::string force_name = assignVarName->getString().str();

    std::cout << "  Registered force assignment: " << force_name << " in " << function_name << "\n";
}

/*
StatementMatcher PosAssignmentInFunction = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl(hasName(function_name))),

        on(hasType(cxxRecordDecl(hasName("Particle")))),

        callee(cxxMethodDecl(hasName("set_pos")))
    ).bind("assignExpr"));

StatementMatcher VelAssignmentInFunction = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl(hasName(function_name))),

        on(hasType(cxxRecordDecl(hasName("Particle")))),

        callee(cxxMethodDecl(hasName("set_vel")))
    ).bind("assignExpr"));

StatementMatcher AccAssignmentInFunction(std::string function_name) {
    return traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl(hasName(function_name))),

        on(hasType(cxxRecordDecl(hasName("Particle")))),

        callee(cxxMethodDecl(hasName("set_acc")))
    ).bind("assignExpr"));
}


// DEPENDENCIES

StatementMatcher ForceDependencyInFunction(std::string function_name) {
    return traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl(hasName(function_name))),

        anyOf(
            callee(cxxMethodDecl(hasName("get_force_vector"))),
            callee(cxxMethodDecl(hasName("get_force_scalar")))
        ),

        hasArgument(0, ignoringImplicit( stringLiteral().bind("dependentVariableName") ))
    ).bind("dependExpr"));
}

StatementMatcher PosDependencyInFunction(std::string function_name) {
    return traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl(hasName(function_name))),

        on(hasType(cxxRecordDecl(hasName("Particle")))),

        callee(cxxMethodDecl(hasName("get_pos")))
    ).bind("dependExpr"));
}

StatementMatcher VelDependencyInFunction(std::string function_name) {
    return traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl(hasName(function_name))),

        on(hasType(cxxRecordDecl(hasName("Particle")))),

        callee(cxxMethodDecl(hasName("get_vel")))
    ).bind("dependExpr"));
}

StatementMatcher AccDependencyInFunction(std::string function_name) {
    return traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl(hasName(function_name))),

        on(hasType(cxxRecordDecl(hasName("Particle")))),

        callee(cxxMethodDecl(hasName("get_acc")))
    ).bind("dependExpr"));
}
*/

}
}