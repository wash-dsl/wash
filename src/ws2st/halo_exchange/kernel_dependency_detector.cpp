#include "kernel_dependency_detector.hpp"

namespace wash {

StatementMatcher AddForceKernelMatcher = traverse(TK_IgnoreUnlessSpelledInSource, callExpr(
        hasAncestor(functionDecl(hasName("main"))),
        hasDescendant(
            declRefExpr(
                to(functionDecl(
                    hasName("wash::add_force_kernel")
                ))
            )
        ),
        hasArgument(0, ignoringImplicit( stringLiteral().bind("kernelName") ))
    ).bind("kernelExpr"));

}