#pragma once

#include <clang-c/Index.h>
#include <iostream>

int main(int argc, char** argv);

std::ostream& operator<<(std::ostream& stream, const CXString& str);