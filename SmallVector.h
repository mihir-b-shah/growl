
#ifndef SMALL_VECTOR_H
#define SMALL_VECTOR_H

#include <vector>

namespace Utils {
    /*based on Chandler Carruth's talk at CppCon 2016 */
    /** Type T, threshold for stack allocation N.*/
    template<typename T, size_t N>
    class SmallVector {
        private:
            bool heap;
            union {
                std::vector<T> vi;
                struct {
                    T* begin;
                    int size;
                } alloc;
            } data;
        public:
            /* I usually try to use consistent case, but here I'll
               try to mimic the standard. */ 
            SmallVector(T* ptr);
            ~SmallVector();
            void push_back(T& val);
            void pop_back();
            T& operator [] (size_t idx) const; 
            size_t size() const;
            T* begin() const;
            T* end() const;
    };
}

#endif