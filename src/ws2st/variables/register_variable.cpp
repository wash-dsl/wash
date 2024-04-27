/**
 * @file register_variable.cpp
 * @author james
 * @brief Implementation for calls to register variables
 * @version 0.1
 * @date 2024-01-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "variables.hpp"

namespace ws2st {

namespace refactor {

namespace variables {

    // void add_variable(const std::string variable, double init_value = 0.0);
    StatementMatcher RegisterVariableMatcher = traverse(TK_IgnoreUnlessSpelledInSource, callExpr(
        hasDescendant(
            declRefExpr(
                to(functionDecl(
                    hasName("wash::add_variable")
                ))
            )
        ),
        hasArgument(0, ignoringImplicit( stringLiteral().bind("variableName") )),
        hasArgument(1, ignoringImplicit( expr().bind("initValue") ))
    ).bind("call"));
    
    StatementMatcher RegisterVariableNoInitMatcher = traverse(TK_IgnoreUnlessSpelledInSource, callExpr(
        hasDescendant(
            declRefExpr(
                to(functionDecl(
                    hasName("wash::add_variable")
                ))
            )
        ),
        hasArgument(0, ignoringImplicit( stringLiteral().bind("variableName") )),
        argumentCountIs(1)
    ).bind("call"));

    void HandleRegisterVariable(const MatchFinder::MatchResult &Result, Replacements& Replace) {
        const auto call = Result.Nodes.getNodeAs<CallExpr>("call");
        const auto variable_name = Result.Nodes.getNodeAs<clang::StringLiteral>("variableName");
        const auto init_value = Result.Nodes.getNodeAs<Expr>("initValue");

        std::string variable_name_str = variable_name->getString().str();

        std::string init_value_str = "0.0";
        if (init_value) {
            init_value_str = getSourceText(Result.Context, init_value->getSourceRange()).value();
        } 

        program_meta->variable_list.push_back(variable_name_str);

        auto replacement_str = "wash::set_variable(\"" + variable_name_str + "\", " + init_value_str + ")";
        auto Err = Replace.add(Replacement(*Result.SourceManager, CharSourceRange::getTokenRange(call->getSourceRange()), replacement_str));
        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
        } else {
            std::cout << "\tRegistered Variable " << variable_name_str << " = " << init_value_str << std::endl;
        }
    }

    std::string getVariableDeclarationSource() {
        std::string outputStr;

        for (auto variable : program_meta->variable_list) {
            outputStr += "extern double variable_" + variable + ";\n";
        }

        return outputStr;
    }

    // acts as initialiser too
    std::string getVariableDefinitionSource() {
        std::string outputStr;

        for (auto variable : program_meta->variable_list) {
            outputStr += "double variable_" + variable + ";\n";
        }

        return outputStr;
    }

    DeclarationMatcher InsertVariablesDeclarationMatcher = traverse(TK_IgnoreUnlessSpelledInSource, 
        cxxRecordDecl(hasName("_variables_defs")).bind("decl")
    );

    void HandleInsertVariablesDeclaration(const MatchFinder::MatchResult &Result, Replacements& Replace) {
        const auto decl = Result.Nodes.getNodeAs<CXXRecordDecl>("decl");

        std::string replacementStr = getVariableDeclarationSource();

        auto Err = Replace.add(Replacement(
            *Result.SourceManager, CharSourceRange::getTokenRange(decl->getSourceRange()), replacementStr));

        if (Err) {
            std::cout << llvm::toString(std::move(Err)) << std::endl;
            throw std::runtime_error("Error handling a match callback.");
        } else {
            std::cout << "Inserted variables definition" << std::endl;
        }
    }

}

}

}