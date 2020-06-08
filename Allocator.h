
#ifndef GLOBAL_H
#define GLOBAL_H

#include <cstdlib>
#include <cstring>
#include <cstdio>

namespace Global {
    class Alloc {
        private:
            void* begin;
            int length;
            int capacity;
            static const int GROWTH_FACTOR = 2;
        public:
            Alloc(int size);
            ~Alloc();
            void printDebug(const char* tag) {
                std::printf("%s:: front: %d length: %d capacity: %d\n", tag, begin, length, capacity);
            }
            
            template<typename T>
            T* allocate(int N) {
                const int offset = ((unsigned long long) (static_cast<unsigned char*>
                                    (Alloc::begin)+Alloc::length))%sizeof(T);
                if(__builtin_expect(sizeof(T)*N+Alloc::length+offset > Alloc::capacity, false)) {
                    // reallocate
                    // should NEVER happen
                    int newsize = static_cast<int>((sizeof(T)*N+Alloc::length)*GROWTH_FACTOR);
                    void* alloc = std::malloc(newsize);
                    std::memcpy(alloc, Alloc::begin, Alloc::length);
                    Alloc::capacity = newsize;
                    std::free(Alloc::begin);
                    Alloc::begin = alloc;
                }
                void* ret = static_cast<unsigned char*>(Alloc::begin)+Alloc::length+offset;
                length += offset+N*sizeof(T);
                return static_cast<T*>(ret);
            }
    };

    Alloc* getAllocator();
}

#endif