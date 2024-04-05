#include "kernel_dependency_detector.hpp"

namespace ws2st {
namespace dependency_detection {

/**
 * @brief Record a force that is assigned to in a previously-registered kernel. Returns true if function_name is a declared kernel, false otherwise.
*/
bool RecordAssignment(std::string function_name, std::string force_name) {
    // Don't need to do anything for the init kernels
    if (std::find(program_meta->init_kernels_list.begin(), program_meta->init_kernels_list.end(), function_name) != program_meta->init_kernels_list.end()) {
        return true;
    }

    if (program_meta->kernels_dependency_map.find(function_name) == program_meta->kernels_dependency_map.end()) {
        std::cerr << "Particle field written to in " << function_name << " which isn't a registered kernel function" << std::endl;
        throw std::runtime_error("Invalid particle field write");
        return false;
    }
        
    std::cout << "Recording write to " << force_name << " in " << function_name << std::endl;

    KernelDependencies* dependencies = program_meta->kernels_dependency_map.at(function_name).get();
    dependencies->writes_to.push_back(force_name);
    return true;
};

/**
 * @brief Record a force that is read from in a previously-registered kernel. Returns true if function_name is a declared kernel, false otherwise.
*/
bool RecordRead(std::string function_name, std::string force_name) {
    if (std::find(program_meta->init_kernels_list.begin(), program_meta->init_kernels_list.end(), function_name) != program_meta->init_kernels_list.end()) {
        return true;
    }
    
    if (program_meta->kernels_dependency_map.find(function_name) == program_meta->kernels_dependency_map.end()) {
        std::cerr << "Particle field read from in " << function_name << " which isn't a registered kernel function" << std::endl;
        throw std::runtime_error("Invalid particle field read");
        return false;
    }

    std::cout << "Recording read of " << force_name << " in " << function_name << std::endl;

    KernelDependencies* dependencies = program_meta->kernels_dependency_map.at(function_name).get();
    dependencies->reads_from.push_back(force_name);
    return true;
}
    

// KERNELS

StatementMatcher AddInitKernelMatcher = traverse(TK_IgnoreUnlessSpelledInSource, callExpr(
    hasAncestor(functionDecl(hasName("main"))),
    hasDescendant(declRefExpr(to(functionDecl(anyOf(
        hasName("wash::add_init_update_kernel"),
        hasName("wash::add_init_void_kernel")
    ))))),
        hasArgument(0, ignoringImplicit(unaryOperator(
        hasOperatorName("&"),
        hasDescendant(
            declRefExpr().bind("kernel")
        )
    ).bind("kernelPtr") ))
).bind("callExpr"));

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
    program_meta->kernels_list.push_back(name);

    // Register a new entry into the dependency table
    auto empty_dependencies = std::make_unique<KernelDependencies>( KernelDependencies{ std::vector<std::string>(), std::vector<std::string>() } );
    program_meta->kernels_dependency_map.insert_or_assign(name, std::move(empty_dependencies));

    std::cout << "  Registered kernel " << name << "\n";
}

void RegisterInitKernel(const MatchFinder::MatchResult &Result, Replacements& Replace) {
    const clang::CallExpr* callExpr = Result.Nodes.getNodeAs<clang::CallExpr>("callExpr");
    const clang::DeclRefExpr* kernel = Result.Nodes.getNodeAs<clang::DeclRefExpr>("kernel");

    if (!callExpr || !kernel) {
        std::cerr << "Init kernel match found without callExpr or kernelPtr" << std::endl;
        throw std::runtime_error("Kernel registration match had no kernel pointer or call node");
    }

    // Get the name of the kernel and register it in our vector
    const std::string name = kernel->getNameInfo().getAsString();
    program_meta->init_kernels_list.push_back(name);

    std::cout << "  Registered init kernel " << name << "\n";
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

    // Insert a new entry into the dependency table
    if (RecordAssignment(function_name, force_name))
        std::cout << "  Registered force assignment: " << force_name << " in " << function_name << "\n";
}


StatementMatcher PosAssignmentInFunction = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl().bind("caller")),

        callee(cxxMethodDecl(hasName("set_pos")))
    ).bind("assignExpr"));

void RegisterPosAssignment(const MatchFinder::MatchResult &Result, Replacements& Replace) {
    const clang::CXXMemberCallExpr *assignExpr = Result.Nodes.getNodeAs<clang::CXXMemberCallExpr>("assignExpr");
    const clang::FunctionDecl *functionDecl = Result.Nodes.getNodeAs<clang::FunctionDecl>("caller");

    if (!assignExpr || !functionDecl) {
        std::cerr << "Match found without assignExpr or functionDecl" << std::endl;
        throw std::runtime_error("Pos assignment match had missing binds");
    }

    const std::string function_name = functionDecl->getNameInfo().getAsString();

    // Insert a new entry into the dependency table
    if (RecordAssignment(function_name, "pos"))
        std::cout << "  Registered force assignment: pos in " << function_name << "\n";
}



StatementMatcher VelAssignmentInFunction = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl().bind("caller")),

        callee(cxxMethodDecl(hasName("set_vel")))
    ).bind("assignExpr"));

void RegisterVelAssignment(const MatchFinder::MatchResult &Result, Replacements& Replace) {
    const clang::CXXMemberCallExpr *assignExpr = Result.Nodes.getNodeAs<clang::CXXMemberCallExpr>("assignExpr");
    const clang::FunctionDecl *functionDecl = Result.Nodes.getNodeAs<clang::FunctionDecl>("caller");

    //assignExpr->dump();

    if (!assignExpr || !functionDecl) {
        std::cerr << "Match found without assignExpr or functionDecl" << std::endl;
        throw std::runtime_error("Vel assignment match had missing binds");
    }

    const std::string function_name = functionDecl->getNameInfo().getAsString();

    // Insert a new entry into the dependency table
    if (RecordAssignment(function_name, "vel"))
        std::cout << "  Registered force assignment: vel in " << function_name << "\n";
}


StatementMatcher AccAssignmentInFunction = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl().bind("caller")),

        callee(cxxMethodDecl(hasName("set_acc")))
    ).bind("assignExpr"));

void RegisterAccAssignment(const MatchFinder::MatchResult &Result, Replacements& Replace) {
    const clang::CXXMemberCallExpr *assignExpr = Result.Nodes.getNodeAs<clang::CXXMemberCallExpr>("assignExpr");
    const clang::FunctionDecl *functionDecl = Result.Nodes.getNodeAs<clang::FunctionDecl>("caller");

    //assignExpr->dump();

    if (!assignExpr || !functionDecl) {
        std::cerr << "Match found without assignExpr or functionDecl" << std::endl;
        throw std::runtime_error("Acc assignment match had missing binds");
    }

    const std::string function_name = functionDecl->getNameInfo().getAsString();

    // Insert a new entry into the dependency table
    if (RecordAssignment(function_name, "acc"))
        std::cout << "  Registered force assignment: acc in " << function_name << "\n";
}


StatementMatcher ForceReadInFunction = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl().bind("caller")),

        anyOf(
            callee(cxxMethodDecl(hasName("get_force_vector"))),
            callee(cxxMethodDecl(hasName("get_force_scalar")))
        ),

        hasArgument(0, ignoringImplicit( stringLiteral().bind("readVariableName") ))
    ).bind("readExpr"));

void RegisterForceRead(const MatchFinder::MatchResult &Result, Replacements& Replace) {
    const clang::CXXMemberCallExpr *readExpr = Result.Nodes.getNodeAs<clang::CXXMemberCallExpr>("readExpr");
    const clang::FunctionDecl *functionDecl = Result.Nodes.getNodeAs<clang::FunctionDecl>("caller");
    const clang::StringLiteral *readVarName = Result.Nodes.getNodeAs<clang::StringLiteral>("readVariableName");

    if (!readExpr || !functionDecl || !readVarName) {
        std::cerr << "Match found without readExpr, functionDecl or kernelPtr" << std::endl;
        throw std::runtime_error("Force assignment match had missing binds");
    }

    const std::string function_name = functionDecl->getNameInfo().getAsString();
    const std::string force_name = readVarName->getString().str();

    // Insert a new entry into the dependency table
    if (RecordRead(function_name, force_name))
        std::cout << "  Registered force read: " << force_name << " in " << function_name << "\n";
}


StatementMatcher PosReadInFunction = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl().bind("caller")),

        callee(cxxMethodDecl(hasName("get_pos")))
    ).bind("readExpr"));

void RegisterPosRead(const MatchFinder::MatchResult &Result, Replacements& Replace) {
    const clang::CXXMemberCallExpr *readExpr = Result.Nodes.getNodeAs<clang::CXXMemberCallExpr>("readExpr");
    const clang::FunctionDecl *functionDecl = Result.Nodes.getNodeAs<clang::FunctionDecl>("caller");

    if (!readExpr || !functionDecl) {
        std::cerr << "Match found without functionDecl or readExpr" << std::endl;
        throw std::runtime_error("Force assignment match had missing binds");
    }

    const std::string function_name = functionDecl->getNameInfo().getAsString();

    // Insert a new entry into the dependency table
    if (RecordRead(function_name, "pos"))
        std::cout << "  Registered force assignment: pos in " << function_name << "\n";
}


StatementMatcher VelReadInFunction = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl().bind("caller")),

        callee(cxxMethodDecl(hasName("get_vel")))
    ).bind("readExpr"));

void RegisterVelRead(const MatchFinder::MatchResult &Result, Replacements& Replace) {
    const clang::CXXMemberCallExpr *readExpr = Result.Nodes.getNodeAs<clang::CXXMemberCallExpr>("readExpr");
    const clang::FunctionDecl *functionDecl = Result.Nodes.getNodeAs<clang::FunctionDecl>("caller");

    if (!readExpr || !functionDecl) {
        std::cerr << "Match found without functionDecl or readExpr" << std::endl;
        throw std::runtime_error("Force assignment match had missing binds");
    }

    const std::string function_name = functionDecl->getNameInfo().getAsString();

    // Insert a new entry into the dependency table
    if (RecordRead(function_name, "vel"))
        std::cout << "  Registered force assignment: vel in " << function_name << "\n";
}


StatementMatcher AccReadInFunction = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl().bind("caller")),

        callee(cxxMethodDecl(hasName("get_acc")))
    ).bind("readExpr"));

void RegisterAccRead(const MatchFinder::MatchResult &Result, Replacements& Replace) {
    const clang::CXXMemberCallExpr *readExpr = Result.Nodes.getNodeAs<clang::CXXMemberCallExpr>("readExpr");
    const clang::FunctionDecl *functionDecl = Result.Nodes.getNodeAs<clang::FunctionDecl>("caller");

    if (!readExpr || !functionDecl) {
        std::cerr << "Match found without functionDecl or readExpr" << std::endl;
        throw std::runtime_error("Force assignment match had missing binds");
    }

    const std::string function_name = functionDecl->getNameInfo().getAsString();

    // Insert a new entry into the dependency table
    if (RecordRead(function_name, "vel"))
        std::cout << "  Registered force assignment: acc in " << function_name << "\n";
}

/*

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