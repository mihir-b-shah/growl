
#ifndef VECTOR_HPP
#define VECTOR_HPP

#include "Error.h"
#include <cstddef>
#include <algorithm>
#include "Allocator.h"

namespace Utils {
    
    // slightly less memory efficient but dont really care
    // convenience and removing possibility of code bloat w/ templates is more imp.
    template<typename T>
    class Vector {
        private:
            static const int FLAG_POS = (sizeof(size_t)*8)-1;
            static const size_t FLAG_SFT = 1ULL << FLAG_POS;
            static const size_t MAX_CAPACITY = (1ULL << FLAG_POS-1)-1;
        protected:
            T* front;
            size_t length;
            size_t capacity; // bit manipulation to encode stack/heap.
            Vector(T* st, size_t l, size_t c){
                if(__builtin_expect(c > MAX_CAPACITY,false)){
                    Global::specifyError("Vector requested more than (1 << sizeof(size_t)-2)-1 capacity.");
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
                enum:char {STACK_PUSH, STACK_MOVE_HEAP, HEAP_PUSH, HEAP_GROW};
                switch((heap<<1)+leqc){
                    case STACK_MOVE_HEAP:
                    case HEAP_GROW:
                    {
                        if(__builtin_expect(length*2 > MAX_CAPACITY,false)){
                            Global::specifyError("Vector requested more than 1 << sizeof(size_t)-2 capacity.");
                            throw Global::MemoryRequestError;
                        }
                        T* aux = Global::getAllocator()->allocate<T>(length*2);
                        std::copy(front, front+capacity, aux); 
                        capacity = FLAG_SFT+length*2;
                        if(__builtin_expect((heap<<1)+leqc==HEAP_GROW,true)){
                            Global::getAllocator()->deallocate<T>(front);
                        }
                        front = aux;
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
    };
    
    /*based on Chandler Carruth's talk at CppCon 2016 */
    /** Type T, threshold for stack allocation N.*/
    template<typename T, size_t N>
    class SmallVector : public Vector<T> {
        private:
            T buffer[N];
        public:
            SmallVector() : Vector<T> (buffer, 0, N) {
            }
            ~SmallVector(){
                if(this->capacity > N){
                    Global::getAllocator()->deallocate<T>(this->front);
                }
            }
    };
}

#endif
