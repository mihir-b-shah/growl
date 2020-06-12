
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
            Alloc(int size); // for now just pass 0.
            ~Alloc();
            void printDebug(const char* tag) {
                std::printf("%s:: front: %d length: %d capacity: %d\n", tag, begin, length, capacity);
            }
            
            // will give these good implementations later.
            // right now, i just want a global point of access.
            template<typename T>
            T* allocate(int N) {
                return new T[N];
            }
            
            template<typename T>
            void deallocate(T* data) {
                delete [] data;
            }
    };

    Alloc* getAllocator();
}

#endif
