
#ifndef SMALL_VECTOR_HPP
#define SMALL_VECTOR_HPP

#include <cstddef>
#include <algorithm>
#include "Allocator.h"

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
            SmallVector(){
                heap = false;
                length = 0;
            }
            ~SmallVector(){
                if(heap){
                    Global::getAllocator()->deallocate<T>(data.alloc.begin);
                }
            }
            void push_back(T val){
                if(heap && length == data.alloc.capacity) {
                    T* aux = Global::getAllocator()->allocate<T>(
                        data.alloc.capacity = length*2);
                    std::copy(data.alloc.begin, N, aux); 
                    Global::getAllocator()->deallocate<T>(data.alloc.begin); 
                    (data.alloc.begin = aux)[length++] = val; 
                } else if(heap) {
                    data.alloc.begin[length++] = val; 
                } else if(length == data.alloc.capacity) {
                    heap = true;
                    T* aux = Global::getAllocator()->allocate<T>(
                        data.alloc.capacity = length*2);
                    std::copy(data.buffer, N, aux); 
                    data.alloc.begin = aux;
                    data.alloc.begin[length++] = val; 
                } else {
                    data.buffer[length++] = val;
                }
            }
            void pop_back(){
                --length;
            }
            T operator [] (int idx) const {
                return heap ? data.alloc.begin[idx] : data.buffer[idx];
            }
            int size() const {
                return length;
            }
            T* begin() const {
                return heap ? data.alloc.begin : data.buffer;
            }
            T* end() const {
                return size+(heap ? data.alloc.begin : data.buffer);
            }
    };
}

#endif