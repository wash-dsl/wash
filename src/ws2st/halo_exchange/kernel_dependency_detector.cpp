#include "kernel_dependency_detector.hpp"

namespace ws2st {
namespace dependency_detection {

/**
 * @brief Record a force that is assigned to in a function.
*/
bool RecordAssignment(std::string function_name, std::string force_name) {
    std::cout << "Function " << function_name << " writes to [" << force_name << "]" << std::endl;

    // If this function was not previously in the dependency map, add it. 
    if (program_meta->kernels_dependency_map.count(function_name) == 0) {
        auto empty_dependencies = std::make_unique<KernelDependencies>( KernelDependencies{ std::vector<std::string>(), std::vector<std::string>() } );
        program_meta->kernels_dependency_map.insert_or_assign(function_name, std::move(empty_dependencies));
    }

    KernelDependencies* dependencies = program_meta->kernels_dependency_map[function_name].get();

    // Don't re-record a dependency we've already recorded
    if (std::find(dependencies->writes_to.begin(), dependencies->writes_to.end(), force_name) == dependencies->writes_to.end()) {
        dependencies->writes_to.push_back(force_name);
    }

    return true;
};

/**
 * @brief Record a force that is read from in function
*/
bool RecordRead(std::string function_name, std::string force_name) {
    std::cout << "Function " << function_name << " reads from [" << force_name << "]" << std::endl;

    // If this was not previously in the dependency map, add it. 
    if (program_meta->kernels_dependency_map.count(function_name) == 0) {
        auto empty_dependencies = std::make_unique<KernelDependencies>( KernelDependencies{ std::vector<std::string>(), std::vector<std::string>() } );
        program_meta->kernels_dependency_map.insert_or_assign(function_name, std::move(empty_dependencies));
    }

    KernelDependencies* dependencies = program_meta->kernels_dependency_map[function_name].get();

    // Don't re-record a dependency we've already recorded
    if (std::find(dependencies->reads_from.begin(), dependencies->reads_from.end(), force_name) == dependencies->reads_from.end()) {
        dependencies->reads_from.push_back(force_name);
    }

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
            ).bind("kernelType")
        ),
        
        hasArgument(0, ignoringImplicit(unaryOperator(
            hasOperatorName("&"),
            hasDescendant(
                declRefExpr().bind("kernel")
            )
        ).bind("kernelPtr") ))
    ).bind("callExpr"));

StatementMatcher SetNeighbourSearchKernelMatcher = traverse(TK_IgnoreUnlessSpelledInSource, callExpr(
        hasAncestor(functionDecl(hasName("main"))),
        hasDescendant(
            declRefExpr(
                to(functionDecl(
                    hasName("wash::set_neighbor_search_kernel")
                ))
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
    const clang::DeclRefExpr *kernelType = Result.Nodes.getNodeAs<clang::DeclRefExpr>("kernelType");


    if (!callExpr || !kernel) {
        std::cerr << "Match found without callExpr or kernelPtr" << std::endl;
        throw std::runtime_error("Kernel registration match had no kernel pointer or call node");
    }

    // Get the name of the kernel and register it in our vector
    const std::string name = kernel->getNameInfo().getAsString();
    program_meta->kernels_list.push_back(name);

    // Figure out if this is actually a force kernel
    const std::string kernelTypeName = kernelType->getNameInfo().getAsString();
    bool is_force_kernel = kernelTypeName == "add_force_kernel";
    program_meta->domain_sync_before.push_back(is_force_kernel);

    // Register a new entry into the dependency table if it's not been seen before
    if (program_meta->kernels_dependency_map.count(name) == 0) {
        auto empty_dependencies = std::make_unique<KernelDependencies>( KernelDependencies{ std::vector<std::string>(), std::vector<std::string>() } );
        program_meta->kernels_dependency_map.insert_or_assign(name, std::move(empty_dependencies));
    }

    std::cout << "  Registered kernel " << name << std::endl;
    if (is_force_kernel) {
        std::cout << "      This is a force kernel" << std::endl;
    }
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

void RegisterNeighbourSearchKernel(const MatchFinder::MatchResult &Result, Replacements& Replace) {
    const clang::CallExpr* callExpr = Result.Nodes.getNodeAs<clang::CallExpr>("callExpr");
    const clang::DeclRefExpr* kernel = Result.Nodes.getNodeAs<clang::DeclRefExpr>("kernel");

    if (!callExpr || !kernel) {
        std::cerr << "Init kernel match found without callExpr or kernelPtr" << std::endl;
        throw std::runtime_error("Kernel registration match had no kernel pointer or call node");
    }

    // Get the name of the kernel and register it
    const std::string name = kernel->getNameInfo().getAsString();
    program_meta->neighbour_kernel = name;

    std::cout << "  Registered neighbour search kernel " << name << "\n";
};

// ANY FUNCTION CALL WITHIN ANOTHER FUNCTION

StatementMatcher GenericFunctionCallInFunction = traverse(TK_IgnoreUnlessSpelledInSource, callExpr(
    hasAncestor(functionDecl().bind("caller"))
).bind("callee"));

void HandleFunctionCallInFunction(const MatchFinder::MatchResult &Result, Replacements &Replace) {
    const clang::CallExpr *callExpr = Result.Nodes.getNodeAs<clang::CallExpr>("callee");
    const clang::FunctionDecl *caller = Result.Nodes.getNodeAs<clang::FunctionDecl>("caller");

    if (!callExpr && !caller) {
        std::cerr << "Match found without a caller or callee" << std::endl;
        throw std::runtime_error("Function call without a caller?");
    }

    const clang::FunctionDecl *calleeFunction = callExpr->getDirectCallee();
    if (!calleeFunction) { // We're probably not interested in this if it doesn't have a function / name
        return;
    }

    std::string calleeName = calleeFunction->getNameInfo().getAsString();
    std::string callerName = caller->getNameInfo().getAsString();

    // if the caller is in the kernel list then we add all the callee's dependencies to the caller's
    bool callerIsKernelFunction = std::find(program_meta->kernels_list.begin(), program_meta->kernels_list.end(), callerName) != program_meta->kernels_list.end();
    bool calleeHasDependencies = program_meta->kernels_dependency_map.count(calleeName) != 0;

    if (callerIsKernelFunction && calleeHasDependencies) {
        std::cout << " Kernel " << callerName << " calls function " << calleeName;

        auto calleeDeps = program_meta->kernels_dependency_map.at(calleeName).get();
        auto callerDeps = program_meta->kernels_dependency_map.at(callerName).get();

        for (auto read_dep : calleeDeps->reads_from) {
            callerDeps->reads_from.push_back(read_dep);
        }

        for (auto write_dep : calleeDeps->writes_to) {
            callerDeps->reads_from.push_back(write_dep);
        }
        
        std::cout << " merged dependencies" << std::endl;
    }
}

// FORCE READs / WRITEs 

StatementMatcher ForceAssignmentInFunction = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl().bind("caller")),

        anyOf(
            callee(cxxMethodDecl(hasName("set_force_vector"))),
            callee(cxxMethodDecl(hasName("set_force_scalar")))
        ),

        hasArgument(0, ignoringImplicit( stringLiteral().bind("assignVariableName") ))
    ).bind("assignExpr")
);

const StatementMatcher WritePropertyMatcher(const std::string propertyName) {
    return traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl().bind("caller")),

        callee(cxxMethodDecl(hasName("set_" + propertyName)))
    ).bind("assignExpr"));
}

StatementMatcher ForceReadInFunction = traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl().bind("caller")),

        anyOf(
            callee(cxxMethodDecl(hasName("get_force_vector"))),
            callee(cxxMethodDecl(hasName("get_force_scalar")))
        ),

        hasArgument(0, ignoringImplicit( stringLiteral().bind("readVariableName") ))
    ).bind("readExpr")
);

const StatementMatcher ReadPropertyMatcher(const std::string propertyName) {
    return traverse(TK_IgnoreUnlessSpelledInSource, cxxMemberCallExpr(
        hasAncestor(functionDecl().bind("caller")),

        callee(cxxMethodDecl(hasName("get_" + propertyName)))
    ).bind("readExpr"));
}

void RegisterForceAssignment(const MatchFinder::MatchResult &Result, Replacements& Replace) {
    const clang::CXXMemberCallExpr *assignExpr = Result.Nodes.getNodeAs<clang::CXXMemberCallExpr>("assignExpr");
    const clang::FunctionDecl *functionDecl = Result.Nodes.getNodeAs<clang::FunctionDecl>("caller");
    const clang::StringLiteral *assignVarName = Result.Nodes.getNodeAs<clang::StringLiteral>("assignVariableName");

    if (!assignExpr || !functionDecl || ! assignVarName) {
        std::cerr << "Force assign match found without caller, assignExpr or force name" << std::endl;
        throw std::runtime_error("Force assign match found without caller, assignExpr or force name");
    }

    const std::string function_name = functionDecl->getNameInfo().getAsString();
    const std::string force_name = assignVarName->getString().str();

    // Insert a new entry into the dependency table
    if (!RecordAssignment(function_name, force_name))
        std::cout << "  Error registering force assignment: " << force_name << " in " << function_name << "\n";
}

void RegisterForceRead(const MatchFinder::MatchResult &Result, Replacements& Replace) {
    const clang::CXXMemberCallExpr *readExpr = Result.Nodes.getNodeAs<clang::CXXMemberCallExpr>("readExpr");
    const clang::FunctionDecl *functionDecl = Result.Nodes.getNodeAs<clang::FunctionDecl>("caller");
    const clang::StringLiteral *readVarName = Result.Nodes.getNodeAs<clang::StringLiteral>("readVariableName");

    if (!readExpr || !functionDecl || !readVarName) {
        std::cerr << "Force read match found without caller, readExpr, or force name" << std::endl;
        throw std::runtime_error("Force read match found without caller, readExpr, or force name");
    }

    const std::string function_name = functionDecl->getNameInfo().getAsString();
    const std::string force_name = readVarName->getString().str();

    // Insert a new entry into the dependency table
    if (!RecordRead(function_name, force_name))
        std::cout << "  Error registering force read: " << force_name << " in " << function_name << "\n";
}

template <PropertyList property>
void RegisterReadProperty(const MatchFinder::MatchResult& Result, Replacements& Replace) {
    const std::string propertyName = getPropertyName(property);
    const clang::CXXMemberCallExpr *readExpr = Result.Nodes.getNodeAs<clang::CXXMemberCallExpr>("readExpr");
    const clang::FunctionDecl *functionDecl = Result.Nodes.getNodeAs<clang::FunctionDecl>("caller");

    if (!readExpr || !functionDecl) {
        std::cerr << "Property read match found without caller or readExpr" << std::endl;
        throw std::runtime_error("Property read match found without caller or readExpr");
    }

    const std::string function_name = functionDecl->getNameInfo().getAsString();

    // Insert a new entry into the dependency table
    if (!RecordRead(function_name, propertyName))
        std::cout << "  Error registering force read: " << propertyName << " in " << function_name << "\n";
}

template <PropertyList property>
void RegisterWriteProperty(const MatchFinder::MatchResult& Result, Replacements& Replace) {
    const std::string propertyName = getPropertyName(property);
    const clang::CXXMemberCallExpr *assignExpr = Result.Nodes.getNodeAs<clang::CXXMemberCallExpr>("assignExpr");
    const clang::FunctionDecl *functionDecl = Result.Nodes.getNodeAs<clang::FunctionDecl>("caller");

    if (!assignExpr || !functionDecl) {
        std::cerr << "Property write match found without caller or readExpr" << std::endl;
        throw std::runtime_error("Property write match found without caller or readExpr");
    }
    const std::string function_name = functionDecl->getNameInfo().getAsString();

    // Insert a new entry into the dependency table
    if (!RecordAssignment(function_name, propertyName))
        std::cout << "  Error registering force assignment: " << propertyName << " in " << function_name << "\n";
}

StatementMatcher PosReadInFunction = ReadPropertyMatcher(getPropertyName(PropertyList::Pos));
StatementMatcher VelReadInFunction = ReadPropertyMatcher(getPropertyName(PropertyList::Vel));
StatementMatcher AccReadInFunction = ReadPropertyMatcher(getPropertyName(PropertyList::Acc));
StatementMatcher DensityReadInFunction = ReadPropertyMatcher(getPropertyName(PropertyList::Density));
StatementMatcher MassReadInFunction = ReadPropertyMatcher(getPropertyName(PropertyList::Mass));
StatementMatcher SmoothingLengthReadInFunction = ReadPropertyMatcher(getPropertyName(PropertyList::SmoothingLength));

WashCallbackFn RegisterPosRead = RegisterReadProperty<PropertyList::Pos>;
WashCallbackFn RegisterVelRead = RegisterReadProperty<PropertyList::Vel>;
WashCallbackFn RegisterAccRead = RegisterReadProperty<PropertyList::Acc>;
WashCallbackFn RegisterDensityRead = RegisterReadProperty<PropertyList::Density>;
WashCallbackFn RegisterMassRead = RegisterReadProperty<PropertyList::Mass>;
WashCallbackFn RegisterSmoothingLengthRead = RegisterReadProperty<PropertyList::SmoothingLength>;

StatementMatcher PosWriteInFunction = WritePropertyMatcher(getPropertyName(PropertyList::Pos));
StatementMatcher VelWriteInFunction = WritePropertyMatcher(getPropertyName(PropertyList::Vel));
StatementMatcher AccWriteInFunction = WritePropertyMatcher(getPropertyName(PropertyList::Acc));
StatementMatcher DensityWriteInFunction = WritePropertyMatcher(getPropertyName(PropertyList::Density));
StatementMatcher MassWriteInFunction = WritePropertyMatcher(getPropertyName(PropertyList::Mass));
StatementMatcher SmoothingLengthWriteInFunction = WritePropertyMatcher(getPropertyName(PropertyList::SmoothingLength));

WashCallbackFn RegisterPosWrite = RegisterWriteProperty<PropertyList::Pos>;
WashCallbackFn RegisterVelWrite = RegisterWriteProperty<PropertyList::Vel>;
WashCallbackFn RegisterAccWrite = RegisterWriteProperty<PropertyList::Acc>;
WashCallbackFn RegisterDensityWrite = RegisterWriteProperty<PropertyList::Density>;
WashCallbackFn RegisterMassWrite = RegisterWriteProperty<PropertyList::Mass>;
WashCallbackFn RegisterSmoothingLengthWrite = RegisterWriteProperty<PropertyList::SmoothingLength>;

const std::string getPropertyName(PropertyList property) {
    switch (property) {
        case PropertyList::Pos:
            return "pos";
        case PropertyList::Vel:
            return "vel";
        case PropertyList::Acc:
            return "acc";
        case PropertyList::Density:
            return "density";
        case PropertyList::Mass:
            return "mass";
        case PropertyList::SmoothingLength:
            return "smoothing_length";
        default:
            return "none";
    }
}

}


}