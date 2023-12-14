#include <iostream>

#include <clang-c/Index.h>

int main(int argc, char** argv) {
    CXIndex index = clang_createIndex(0, 0);
    CXTranslationUnit unit = clang_parseTranslationUnit(index, src_file, nullptr, 0, nullptr, 0, CXTranslationUnit_None);

    if (unit == nulltpr) {
        std::cerr << "Couldn't parse this one" << std::endl;
        exit(-1);
    }

    std::cout << "Parsed this one" << std::endl;

    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);
}