
#include <cstdlib>
#include <cstring>
#include "Global.h"

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

template<typename T>
T* Global::Alloc::allocate(int N) {
    if(__builtin_expect(sizeof(T)*N+Global::Alloc::length > Global::Alloc::capacity, false)) {
        // reallocate
        // should NEVER happen
        int newsize = static_cast<int>(capacity*1.5);
        void* alloc = std::malloc(newsize);
        std::memcpy(alloc, Global::Alloc::begin, length);
        Global::Alloc::capacity = newsize;
        std::free(Global::Alloc::begin);
        Global::Alloc::begin = alloc;
    }
    void* ret = static_cast<unsigned char*>(begin)+length;
    length += N;
    return static_cast<T>(ret);
}