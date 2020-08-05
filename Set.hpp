

#ifndef SET_H
#define SET_H

#include <cstddef>
#include <algorithm>
#include "Error.h"

/*
This currently just uses a very simple quadratic-probing scheme.
*/

namespace Utils {

    template<typename T>
    struct SetTraits {
        static T emptyVal();
        static T tombstoneVal();
        static size_t hash(const T x);
        static bool equal(const T x, const T y);
    };

    // write out specializations a user might want.
    template<>
    struct SetTraits<int> {
        static constexpr int emptyVal(){return 0;}
        static constexpr int tombstoneVal(){return -1;}
        static size_t hash(const int x){return x;}
        static bool equal(const int x, const int y){return x==y;}
    };

    template<typename T, typename Traits = SetTraits<T>>
    class Set {
        private:
            static constexpr float LOAD_FACTOR = 0.5;
            static constexpr int FLAG_POS = (sizeof(size_t)*8)-1;
            static constexpr size_t FLAG_SFT = 1ULL << FLAG_POS;
            static constexpr size_t MAX_CAPACITY = (1ULL << FLAG_POS-1)-1;
            static constexpr int GenericError = 0xF; // to be removed.
            enum:char {STACK_PUSH, STACK_MOVE_HEAP, HEAP_PUSH, HEAP_GROW};
        protected:
            T* front;
            size_t capacity;
            size_t _size;
            
            Set(T* st, size_t s, size_t c){
                if(__builtin_expect(c > MAX_CAPACITY,false)){
                    Global::specifyError("Too much memory req.\n", __FILE__, __LINE__);
                    throw Global::MemoryRequestError; 
                }
                front = st;
                _size = s;
                capacity = c;
            }
            ~Set(){
            }
            
            bool fastInsert(T key, T* _table, size_t _capacity){
                size_t idx = Traits::hash(key) & _capacity-1;
                size_t jmp = 0;
                while(!Traits::equal(_table[idx], Traits::emptyVal()) 
                                && !Traits::equal(_table[idx], Traits::tombstoneVal()) 
                                && !Traits::equal(_table[idx], key)){
                    idx += 2*jmp+1;
                    ++jmp;
                    idx &= _capacity - 1; // to optimize away, just getting the easy way first.
                }
                if(__builtin_expect(Traits::equal(_table[idx], key), false)){
                    return false;
                }
                _table[idx] = key;
                return true;
            }
            
        public:
            /* generally if T is big you should store it as a pointer
            anyway, else you should pay the value copy cost. */
            
            bool insert(T key){
                const bool heap = capacity>>FLAG_POS;
                const int realCapacity = capacity&FLAG_SFT-1;
                const bool leqc = _size == static_cast<int>(LOAD_FACTOR*realCapacity);

                switch((heap<<1)+leqc){
                    case STACK_MOVE_HEAP:
                    case HEAP_GROW:
                    {
                        if(__builtin_expect(realCapacity*2 > MAX_CAPACITY,false)){
                            Global::specifyError("Too much memory req.\n", __FILE__, __LINE__);
                            throw Global::MemoryRequestError; 
                        }
                        
                        T* aux = new T[realCapacity*2]();
                        for(int i = 0; i<realCapacity; ++i){
                            if(!Traits::equal(front[i], Traits::emptyVal()) 
                                            && !Traits::equal(front[i], Traits::tombstoneVal())){
                                // guaranteed to fit.
                                fastInsert(front[i], aux, realCapacity*2);
                            }
                        }
                        
                        capacity = FLAG_SFT+realCapacity*2;
                        if(__builtin_expect((heap<<1)+leqc==HEAP_GROW,true)){
                            delete front;
                        }
                        front = aux;
                    }
                    case STACK_PUSH:
                    case HEAP_PUSH:
                        ++_size;
                        return fastInsert(key, front, capacity & FLAG_SFT-1);
                    // no default case.
                }
                
                return false;
            }
            
            // returns nullptr if not found.
            T* find(T key){
                const size_t realCapacity = capacity & FLAG_SFT-1;
                size_t idx = Traits::hash(key) & realCapacity-1;
                size_t jmp = 0;
                while(!Traits::equal(front[idx], Traits::emptyVal()) && 
                                !Traits::equal(front[idx], key)){
                    idx += 2*jmp+1;
                    ++jmp;
                    idx &= realCapacity-1; // to optimize away, just getting the easy way first.
                }
                return Traits::equal(front[idx], key) ? front+idx : nullptr;
            }
            
            bool erase(T key){
                T* ptr = find(key);
                *ptr = Traits::tombstoneVal();
                return ptr != nullptr;
            }

            size_t size(){
                return _size;
            }
    };
    
    /*based on Chandler Carruth's talk at CppCon 2016 */
    /** Type T, threshold for stack allocation N. */
    template<typename T, size_t N, typename Traits = SetTraits<T>>
    class SmallSet : public Set<T, Traits> {
        private:
            T buffer[N] = {Traits::emptyVal()};
        public:
            SmallSet() : Set<T, Traits> (buffer, 0, N) {
            }
            ~SmallSet(){
                if(this->capacity > N){
                    delete this->front;
                }
            }
    };
}

#endif
