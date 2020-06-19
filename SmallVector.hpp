
#ifndef SMALL_VECTOR_HPP
#define SMALL_VECTOR_HPP

#include <cstddef>
#include <algorithm>
#include "Allocator.h"

namespace Utils {
    
    // slightly less memory efficient but dont really care
    // convenience and removing possibility of code bloat w/ templates is more imp.
    template<typename T>
    class Vector {
        private:
            static const int FLAG_POS = sizeof(size_t)-1;
            static const long long FLAG_SFT = 1ULL << FLAG_POS;
        protected:
            T* front;
            size_t length;
            size_t capacity; // bit manipulation to encode stack/heap.
            Vector(T* st, size_t l, size_t c){
                front = st;
                length = l;
                capacity = c;
            }
            ~Vector(){
            }
        public:
            void push_back(T val){
                const bool heap = capacity>>FLAG_POS;
                if(__builtin_expect(heap && length == capacity&FLAG_SFT,false)) {
                    //std::cout << "Heap grow.\n";
                    T* aux = Global::getAllocator()->allocate<T>(capacity = FLAG_SFT+length*2);
                    std::copy(front, front+length, aux); 
                    Global::getAllocator()->deallocate<T>(front); 
                    (front = aux)[length++] = val; 
                } else if(heap) {
                    //std::cout << "Heap push back.\n";
                    front[length++] = val; 
                } else if(__builtin_expect(length == capacity,false)) {
                    //std::cout << "Stack to heap.\n";
                    T* aux = Global::getAllocator()->allocate<T>(length*2);
                    std::copy(front, front+capacity, aux); 
                    front = aux;
                    capacity = FLAG_SFT+length*2;
                    front[length++] = val; 
                } else {
                    //std::cout << "Stack grow.\n";
                    front[length++] = val;
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