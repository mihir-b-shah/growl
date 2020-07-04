
#ifndef QUEUE_HPP
#define QUEUE_HPP

#include "Error.h"
#include "Allocator.h"
#include <cstddef>
#include <algorithm>

namespace Utils {
    template<typename T>
    class Queue {
        private:
            static const int FLAG_POS = (sizeof(size_t)*8)-1;
            static const size_t FLAG_SFT = 1ULL << FLAG_POS;
            static const size_t MAX_CAPACITY = (1ULL << FLAG_POS-1)-1;
        protected:
            T* start;
            size_t _front;
            size_t _back;
            size_t length;
            size_t capacity; // bit manipulation to encode stack/heap.
            
            Queue(T* st, size_t f, size_t b, size_t l, size_t c){
                if(__builtin_expect(c > MAX_CAPACITY,false)){
                    Global::specifyError("Queue requested more than 1 << sizeof(size_t)-2 capacity.");
                    throw Global::MemoryRequestError;
                }
                _front = f;
                _back = b;
                start = st;
                length = l;
                capacity = c;
            }
            ~Queue(){
            }
        public:
            // no code reuse with Vector, too many different things.
            void push_back(T val){
                const bool heap = capacity>>FLAG_POS;
                const bool leqc = (length&FLAG_SFT-1) >0 && _front == _back;
                enum:char {STACK_PUSH, STACK_MOVE_HEAP, HEAP_PUSH, HEAP_GROW};
                
                switch((heap<<1)+leqc){
                    case STACK_MOVE_HEAP:
                    case HEAP_GROW:
                    {
                        if(__builtin_expect(length*2 > MAX_CAPACITY,false)){
                            Global::specifyError("Vector requested more than (1 << sizeof(size_t)-2)-1 capacity.");
                            throw Global::MemoryRequestError;
                        }
                        T* aux = Global::getAllocator()->allocate<T>(length*2);
                        std::copy(start+_front, start+capacity, aux);
                        std::copy(start, start+_back, aux+capacity-_front);
                        capacity = FLAG_SFT+length*2;
                        if(__builtin_expect((heap<<1)+leqc==HEAP_GROW,true)){
                            Global::getAllocator()->deallocate<T>(start);
                        }
                        start = aux;
                        _front = 0;
                        _back = length;
                    }
                    case STACK_PUSH:
                    case HEAP_PUSH:
                        ++length;
                        start[_back++] = val;
                        _back &= capacity-1;
                        break;
                    // no default case.
                }
            }
            void pop_front(){
                --length;
                ++_front;
                _front &= capacity-1;
            }
            int size() const {
                return length;
            }
            T front() const {
                return start[_front];
            }
    };
      
    /*based on Chandler Carruth's talk at CppCon 2016 */
    /** Type T, threshold for stack allocation N.*/
    
    template<typename T, size_t N>
    class SmallQueue : public Queue<T> {
        private:
            static constexpr size_t nextPow2(const size_t curr){
                size_t s = curr;
                --s;
                s |= s >> 1;
                s |= s >> 2;
                s |= s >> 4;
                s |= s >> 8;
                return s+1;
            }
            // should get replaced with literal value.
            T buffer[nextPow2(N)];
        public:
            SmallQueue() : Queue<T> (buffer, 0, 0, 0, nextPow2(N)) {
            }
            ~SmallQueue(){
                if(this->capacity > nextPow2(N)){
                    Global::getAllocator()->deallocate<T>(this->start);
                }
            }
    };
}

#endif

