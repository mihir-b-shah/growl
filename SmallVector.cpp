
#include <algorithm>
#include <cstddef>
#include "SmallVector.h"
#include "Allocator.h"

using namespace Utils;

template<typename T, size_t N>
SmallVector<T,N>::SmallVector(){
    heap = false;
    length = 0;
}

template<typename T, size_t N>
SmallVector<T,N>::~SmallVector(){
    if(heap){
        Global::getAllocator()->deallocate<T>(data.alloc.begin);
    }
}

template<typename T, size_t N>
void SmallVector<T,N>::push_back(T val){
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

/** 
Does not call the destructor on the object at the back.
*/
template<typename T, size_t N>
void SmallVector<T,N>::pop_back(){
    --length;
}

template<typename T, size_t N>
T SmallVector<T,N>::operator [] (int idx) const {
    return heap ? data.alloc.begin[idx] : data.buffer[idx];
}

template<typename T, size_t N>
int SmallVector<T,N>::size() const {
    return length;
}

template<typename T, size_t N>
T* SmallVector<T,N>::begin() const {
    return heap ? data.alloc.begin : data.buffer;
}

template<typename T, size_t N>
T* SmallVector<T,N>::end() const {
    return size+(heap ? data.alloc.begin : data.buffer);
}
