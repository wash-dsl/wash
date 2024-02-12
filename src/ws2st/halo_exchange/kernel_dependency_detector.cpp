#include "kernel_dependency_detector.hpp"

namespace wash {
namespace dependency_detection {


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
        
        hasArgument(0, ignoringImplicit( stringLiteral().bind("kernelName") ))
    ).bind("callExpr"));

std::vector<std::string> kernels;

void HandleRegisterForceKernel(const MatchFinder::MatchResult &Result) {
    const clang::CallExpr *callExpr = Result.Nodes.getNodeAs<clang::CallExpr>("callExpr");
    const clang::StringLiteral *kernelName = Result.Nodes.getNodeAs<clang::StringLiteral>("kernelName");

    if (!callExpr || !kernelName) {
        std::cerr << "Match found without callExpr or kernelName" << std::endl;
        throw std::runtime_error("Kernel registration match had no kernel name or call node");
    }

    std::cout << kernelName->getString().str() << "\n";

}

StatementMatcher ForceAssignmentInFunction(std::string function_name) {
    return traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl(hasName(function_name))),

        anyOf(
            callee(cxxMethodDecl(hasName("set_force_vector"))),
            callee(cxxMethodDecl(hasName("set_force_scalar")))
        ),

        hasArgument(0, ignoringImplicit( stringLiteral().bind("assignVariableName") ))
    ).bind("assignExpr"));
}

StatementMatcher PosAssignmentInFunction(std::string function_name) {
    return traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl(hasName(function_name))),

        on(hasType(cxxRecordDecl(hasName("Particle")))),

        callee(cxxMethodDecl(hasName("set_pos")))
    ).bind("assignExpr"));
}

StatementMatcher VelAssignmentInFunction(std::string function_name) {
    return traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl(hasName(function_name))),

        on(hasType(cxxRecordDecl(hasName("Particle")))),

        callee(cxxMethodDecl(hasName("set_vel")))
    ).bind("assignExpr"));
}

StatementMatcher AccAssignmentInFunction(std::string function_name) {
    return traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl(hasName(function_name))),

        on(hasType(cxxRecordDecl(hasName("Particle")))),

        callee(cxxMethodDecl(hasName("set_acc")))
    ).bind("assignExpr"));
}



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


}
}