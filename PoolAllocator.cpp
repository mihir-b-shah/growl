
#include <cstdlib>
#include <cstring>
#include "Allocator.h"

// this is prob horribly inefficient
using Global::Alloc;

// for now, the allocator is just a placeholder.
Alloc::Alloc(int size) {
    /*
    void* alloc = std::malloc(size);
    begin = alloc;
    length = 0;
    capacity = size;*/
}

Alloc::~Alloc() {
    //std::free(begin);
}

