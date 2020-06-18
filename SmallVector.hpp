
#ifndef SMALL_VECTOR_HPP
#define SMALL_VECTOR_HPP

#include <cstddef>
#include <algorithm>
#include "Allocator.h"

namespace Utils {
    
    template<typename T>
    class Vector {
        public:
            virtual void push_back(T val);
            virtual void pop_back();
            virtual int size() const;
            virtual const T* cbegin() const;
            virtual const T* cend() const;
            virtual T* begin() const;
            virtual T* end() const;
            virtual T operator [] (int idx) const;
            virtual T* back() const;
            virtual T eback() const;
            virtual const T* cback() const;
    };
    
    /*based on Chandler Carruth's talk at CppCon 2016 */
    /** Type T, threshold for stack allocation N.*/
    template<typename T, size_t N>
    class SmallVector : public Vector<T> {
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
                    std::copy(data.alloc.begin, data.alloc.begin+length, aux); 
                    Global::getAllocator()->deallocate<T>(data.alloc.begin); 
                    (data.alloc.begin = aux)[length++] = val; 
                } else if(heap) {
                    data.alloc.begin[length++] = val; 
                } else if(length == N) {
                    heap = true;
                    T* aux = Global::getAllocator()->allocate<T>(length*2);
                    std::copy(data.buffer, data.buffer+N, aux); 
                    data.alloc.begin = aux;
                    data.alloc.capacity = length*2;
                    data.alloc.begin[length++] = val; 
                } else {
                    data.buffer[length++] = val;
                }
            }
            void pop_back(){
                --length;
            }
            int size() const {
                return length;
            }
            const T* cbegin() const {
                return heap ? data.alloc.begin : data.buffer;
            }
            const T* cend() const {
                return length+begin();
            }
            T* begin() const {
                return heap ? data.alloc.begin : data.buffer;
            }
            T* end() const {
                return length+begin();
            }
            T operator [] (int idx) const {
                return *(begin()+idx);
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
}

#endif