
#ifndef SMALL_VECTOR_H
#define SMALL_VECTOR_H

#include <cstddef>

namespace Utils {
    /*based on Chandler Carruth's talk at CppCon 2016 */
    /** Type T, threshold for stack allocation N.*/
    template<typename T, size_t N>
    class SmallVector {
        private:
            bool heap;
            union {
                T buffer[N];
                struct {
                    T* begin;
                    int capacity;
                } alloc;
            } data;
            int length;
        public:
            /* I usually try to use consistent case, but here I'll
               try to mimic the standard. */ 
            SmallVector();
            ~SmallVector();
            void push_back(T val);
            void pop_back();
            T operator [] (int idx) const; 
            int size() const;
            T* begin() const;
            T* end() const;
    };
}

#endif