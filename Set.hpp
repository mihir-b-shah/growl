
#ifndef MAP_H
#define MAP_H

#include <cstddef>
#include <algorithm>

namespace Utils {
    
    // slightly less memory efficient but dont really care
    // convenience and removing possibility of code bloat w/ templates is more imp.
    template<typename T>
    class Set {
       
    };
    
    /*based on Chandler Carruth's talk at CppCon 2016 */
    /** Type T, threshold for stack allocation N.*/
    template<typename T, size_t N>
    class SmallSet : public Set<T> {
        private:
            T buffer[N];
        public:
            SmallSet() : Set<T> (buffer, 0, N) {
            }
            ~SmallSet(){
                if(this->capacity > N){
                    delete this->front;
                }
            }
    };
}

#endif