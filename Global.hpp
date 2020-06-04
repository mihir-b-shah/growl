
#ifndef GLOBAL_H
#define GLOBAL_H

#include <cstdlib>
#include <cstring>
#include <cstdio>

namespace Global {
    const int DeveloperError = 0xF;
    const int InvalidEscapeSequence = 0x10;
    const int InvalidCharacter = 0x11;
    const int InvalidLiteral = 0x12;
    const int InvalidIdentifier = 0x13; // this could also point to an invalid literal
                                        // see the TokenGenerator.cpp where its thrown
    const int InvalidOperator = 0x14; 

    const int ERROR_BUFFER_SIZE = 255;
    const int ERROR_REFERENCE_SIZE = 31;

    static const int GROWTH_FACTOR = 2;

    extern char errorMsg[ERROR_REFERENCE_SIZE];

    void genError(char buffer[ERROR_BUFFER_SIZE], int error);
    void specifyError(const char* spec);

    class Alloc {
        private:
            void* begin;
            int length;
            int capacity;
            static const int getOffset(int size, void* ptr) {
                return (unsigned long long) ptr%size;
            }
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