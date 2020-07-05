

#ifndef SET_H
#define SET_H

#include <cstddef>
#include <algorithm>

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
        static int emptyVal(){return 0;}
        static int tombstoneVal(){return -1;}
        static size_t hash(const int x){return x;}
        static bool equal(const int x, const int y){return x==y;}
    };

    template<typename T, typename Traits = SetTraits<T>>
    class Set {
        private:
            static const int FLAG_POS = (sizeof(size_t)*8)-1;
            static const size_t FLAG_SFT = 1ULL << FLAG_POS;
            static const size_t MAX_CAPACITY = (1ULL << FLAG_POS-1)-1;
            static const int GenericError = 0xF; // to be removed.
        protected:
            T* front;
            size_t capacity;
            size_t _size;
            
            Set(T* st, size_t s, size_t c){
                if(__builtin_expect(c > MAX_CAPACITY,false)){
                    throw GenericError;
                }
                front = st;
                _size = s;
                capacity = c;
            }
            ~Set(){
            }
        public:
            /* generally if T is big you should store it as a pointer
            anyway, else you should pay the value copy cost. */
            
            void insert(T key){
                
            }
            void erase(T key){
                
            }
            
            // returns nullptr if not found.
            T* find(T key){
                
            }
    };
    
    /*based on Chandler Carruth's talk at CppCon 2016 */
    /** Type T, threshold for stack allocation N. */
    template<typename T, size_t N, typename Traits = SetTraits<T>>
    class SmallSet : public Set<T, Traits> {
        private:
            T buffer[N];
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