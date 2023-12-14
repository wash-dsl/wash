/**
 * @file inspect.cpp
 * @author jamesm2w
 * @brief Inspecting the contents of WaSH C++ files
 * @version 0.1
 * @date 2023-12-14
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "inspect.hpp"

int main(int argc, char** argv) {
    std::string src_file;
    if (argc < 2) {
        src_file = "./src/examples/ca_fluid_sim/kernels.hpp";
    } else {
        src_file = argv[1];
    }

    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit unit =
        clang_parseTranslationUnit(index, src_file.c_str(), nullptr, 0, nullptr, 0, CXTranslationUnit_None);

    if (unit == nullptr) {
        std::cerr << "Couldn't parse " << src_file << std::endl;
        exit(-1);
    }

    std::cout << "Successfully Parsed " << src_file << std::endl;

    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(
        cursor,
        [](CXCursor c, CXCursor parent, CXClientData client_data) {
            std::cout << "Cursor \"" << clang_getCursorSpelling(c) << "\" is kind \""
                      << clang_getCursorKindSpelling(clang_getCursorKind(c)) << "\"" << std::endl;
            return CXChildVisit_Recurse;
        },
        nullptr
    );

    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);
}

/**
 * @brief Helper to output Clang strings nicely
 *
 * @param stream
 * @param str
 * @return std::ostream&
 */
std::ostream& operator<<(std::ostream& stream, const CXString& str) {
    stream << clang_getCString(str);
    clang_disposeString(str);
    return stream;
}