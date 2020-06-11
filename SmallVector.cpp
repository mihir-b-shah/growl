
#include <vector>
#include <algorithm>
#include <iostream>
#include "SmallVector.h"

template<typename T, size_t N>
Utils::SmallVector<T,N>::SmallVector(T* ptr){
    Utils::SmallVector<T,N>::heap = false;
    Utils::SmallVector<T,N>::data.alloc.begin = ptr;
    Utils::SmallVector<T,N>::data.alloc.size = 0;
}

template<typename T, size_t N>
Utils::SmallVector<T,N>::~SmallVector(){
}

template<typename T, size_t N>
void Utils::SmallVector<T,N>::push_back(T& val){
    if(__builtin_expect(Utils::SmallVector<T,N>::size == N, false)) {
        // switch over to the heap
        Utils::SmallVector<T,N>::heap = true;
        Utils::SmallVector<T,N>::data.vi.resize(Utils::SmallVector<T,N>::data.alloc.size);
        std::copy(Utils::SmallVector<T,N>::data.alloc.begin, 
            Utils::SmallVector<T,N>::data.alloc.begin+Utils::SmallVector<T,N>::data.alloc.size, Utils::SmallVector<T,N>::vi.begin()); 
    } else if(heap){
        // stay on the heap
        Utils::SmallVector<T,N>::data.vi.push_back(val);
    } else {
        // stay on the stack
        Utils::SmallVector<T,N>::data.alloc.begin[Utils::SmallVector<T,N>::size] = val;
    }
}

template<typename T, size_t N>
void Utils::SmallVector<T,N>::pop_back(){
    if(Utils::SmallVector<T,N>::heap) {
        Utils::SmallVector<T,N>::data.vi.pop_back();
    } else {
        --Utils::SmallVector<T,N>::data.alloc.size;
    }
}

template<typename T, size_t N>
T& Utils::SmallVector<T,N>::operator [] (size_t idx) const {
    if(Utils::SmallVector<T,N>::heap) {
        return Utils::SmallVector<T,N>::data.vi[idx];
    } else {
        return Utils::SmallVector<T,N>::data.alloc.begin[idx];
    }
}

template<typename T, size_t N>
size_t Utils::SmallVector<T,N>::size() const {
    if(Utils::SmallVector<T,N>::heap) {
        return Utils::SmallVector<T,N>::data.vi.size();
    } else {
        return Utils::SmallVector<T,N>::data.alloc.size;
    }
}

template<typename T, size_t N>
T* Utils::SmallVector<T,N>::begin() const {
    if(Utils::SmallVector<T,N>::heap) {
        return Utils::SmallVector<T,N>::data.vi.begin();
    } else {
        return Utils::SmallVector<T,N>::data.alloc.begin;
    }
}

template<typename T, size_t N>
T* Utils::SmallVector<T,N>::end() const {
    if(Utils::SmallVector<T,N>::heap) {
        return Utils::SmallVector<T,N>::data.vi.end();
    } else {
        return Utils::SmallVector<T,N>::data.alloc.begin+Utils::SmallVector<T,N>::data.alloc.size;
    }
}

int main() {
    Utils::SmallVector<int,50> vect;
    vect.push_back(1);
    vect.push_back(2);
    vect.push_back(3);
}