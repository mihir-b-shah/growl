
#ifndef VECTOR_HPP
#define VECTOR_HPP

#include "Error.h"
#include <cstddef>
#include <algorithm>

namespace Utils {
    
    // slightly less memory efficient but dont really care
    // convenience and removing possibility of code bloat w/ templates is more imp.
    template<typename T>
    class Vector {
        private:
            static const int FLAG_POS = (sizeof(size_t)*8)-1;
            static const size_t FLAG_SFT = 1ULL << FLAG_POS;
            static const size_t MAX_CAPACITY = (1ULL << FLAG_POS-1)-1;
            enum:char {STACK_PUSH, STACK_MOVE_HEAP, HEAP_PUSH, HEAP_GROW};
            
            void grow(bool heap, bool leqc){
                if(__builtin_expect(length*2 > MAX_CAPACITY,false)){
                    Global::specifyError("Vector requested more than 1 << sizeof(size_t)-2 capacity.", __FILE__, __LINE__);
                    throw Global::MemoryRequestError;
                }
                T* aux = new T[length*2]();
                std::copy(front, front+capacity, aux); 
                capacity = FLAG_SFT+length*2;
                if(__builtin_expect((heap<<1)+leqc==HEAP_GROW,true)){
                    delete front;
                }
                front = aux;
            }

        protected:
            T* front;
            size_t length;
            size_t capacity; // bit manipulation to encode stack/heap.
            Vector(T* st, size_t l, size_t c){
                if(__builtin_expect(c > MAX_CAPACITY,false)){
                    Global::specifyError("Vector requested more than (1 << sizeof(size_t)-2)-1 capacity.", __FILE__, __LINE__);
                    throw Global::MemoryRequestError;
                }
                front = st;
                length = l;
                capacity = c;
            }
            ~Vector(){
            }
        public:
            void push_back(T val){
                const bool heap = capacity>>FLAG_POS;
                const bool leqc = length == (capacity&FLAG_SFT-1);
                switch((heap<<1)+leqc){
                    case STACK_MOVE_HEAP:
                    case HEAP_GROW:
                    {
                        grow(heap, leqc);
                    }
                    case STACK_PUSH:
                    case HEAP_PUSH:
                        front[length++] = val;
                        break;
                    // no default case.
                }
            }
            void pop_back(){
                --length;
            }
            int size() const {
                return length;
            }
            const T* cbegin() const {
                return front;
            }
            const T* cend() const {
                return length+begin();
            }
            T* begin() const {
                return front;
            }
            T* end() const {
                return front+length;
            }
            T at(int idx) const {
                return *(begin()+idx);
            }
            T operator [] (int idx) const {
                return at(idx);
            }
            T* back() const {
                return length-1+begin();
            }
            T eback() const {
                return *back();
            }
            const T* cback() const {
                return length-1+begin();
            }
            /** Try not to resize beyond the small buffer capacity. */
            void resize(int newSize){
                if(__builtin_expect(newSize > (capacity & FLAG_SFT-1), true)){
                    const bool heap = capacity>>FLAG_POS;
                    const bool leqc = length == (capacity&FLAG_SFT-1);
                    grow(heap, leqc);
                }
                length = newSize;
            }
            void allocate(int howMany){
                resize(length+howMany);
            }
            T& ref(unsigned int idx){
                return *(begin() + idx);
            }
            void swap(unsigned i1, unsigned i2){
                T _swap = *(begin()+i1);
                *(begin()+i1) = *(begin()+i2);
                *(begin()+i2) = _swap;
            }

    };
    
    /*based on Chandler Carruth's talk at CppCon 2016 */
    /** Type T, threshold for stack allocation N.*/
    template<typename T, size_t N>
    class SmallVector : public Vector<T> {
        private:
            T buffer[N] = {T()};
        public:
            SmallVector() : Vector<T> (buffer, 0, N) {
            }
            ~SmallVector(){
                if(this->capacity > N){
                    delete this->front;
                }
            }
    };
}

#endif
