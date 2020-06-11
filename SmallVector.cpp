
#include <vector>
#include <algorithm>
#include <iostream>
#include "SmallVector.h"

using namespace Utils;

template<typename T, size_t N>
SmallVector<T,N>::SmallVector(T* ptr){
    heap = false;
    data.alloc.begin = ptr;
    data.alloc.size = 0;
}

template<typename T, size_t N>
SmallVector<T,N>::~SmallVector(){
}

template<typename T, size_t N>
void SmallVector<T,N>::push_back(T& val){
    if(__builtin_expect(size == N, false)) {
        // switch over to the heap
        heap = true;
        data.vi.resize(data.alloc.size);
        std::copy(data.alloc.begin, data.alloc.begin+data.alloc.size, data.vi.begin()); 
    } else if(heap){
        // stay on the heap
        data.vi.push_back(val);
    } else {
        // stay on the stack
        data.alloc.begin[size] = val;
    }
}

template<typename T, size_t N>
void SmallVector<T,N>::pop_back(){
    if(heap) {
        data.vi.pop_back();
    } else {
        --data.alloc.size;
    }
}

template<typename T, size_t N>
T& SmallVector<T,N>::operator [] (size_t idx) const {
    if(heap) {
        return data.vi[idx];
    } else {
        return data.alloc.begin[idx];
    }
}

template<typename T, size_t N>
size_t SmallVector<T,N>::size() const {
    if(heap) {
        return data.vi.size();
    } else {
        return data.alloc.size;
    }
}

template<typename T, size_t N>
T* SmallVector<T,N>::begin() const {
    if(heap) {
        return data.vi.begin();
    } else {
        return data.alloc.begin;
    }
}

template<typename T, size_t N>
T* SmallVector<T,N>::end() const {
    if(heap) {
        return data.vi.end();
    } else {
        return data.alloc.begin+data.alloc.size;
    }
}
/*
int main() {
    SmallVector<int,50> vect;
    vect.push_back(1);
    vect.push_back(2);
    vect.push_back(3);
}*/