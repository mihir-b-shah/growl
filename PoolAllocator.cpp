
#include <cstdlib>
#include <cstring>
#include "Global.hpp"

// this is prob horribly inefficient

Global::Alloc::Alloc(int size) {
    void* alloc = std::malloc(size);
    Global::Alloc::begin = alloc;
    Global::Alloc::length = 0;
    Global::Alloc::capacity = size;
}

Global::Alloc::~Alloc() {
    std::free(begin);
}
