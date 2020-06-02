
#ifndef GLOBAL_H
#define GLOBAL_H

#include <cstdlib>
#include <cstring>

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

    extern char errorMsg[ERROR_REFERENCE_SIZE];

    void genError(char buffer[ERROR_BUFFER_SIZE], int error);
    void specifyError(const char* spec);

    class Alloc {
        private:
            void* begin;
            int length;
            int capacity;
        public:
            Alloc(int size);
            ~Alloc();

            template<typename T>
            T* allocate(int N) {
                if(__builtin_expect(sizeof(T)*N+Alloc::length > Alloc::capacity, false)) {
                    // reallocate
                    // should NEVER happen
                    int newsize = static_cast<int>(capacity*1.5);
                    void* alloc = std::malloc(newsize);
                    std::memcpy(alloc, Alloc::begin, length);
                    Alloc::capacity = newsize;
                    std::free(Alloc::begin);
                    Alloc::begin = alloc;
                }
                void* ret = static_cast<unsigned char*>(begin)+length;
                length += N;
                return static_cast<T*>(ret);
            }
    };

    Alloc* getAllocator();
}

#endif