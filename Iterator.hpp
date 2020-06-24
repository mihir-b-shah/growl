
#ifndef ITERATOR_H
#define ITERATOR_H

namespace Utils {
    template<typename T>
    class Iterator {
        protected:
            T* curr;
        public:
            T operator*();
            Iterator operator++();
            Iterator operator+(Iterator iter);
            Iterator operator-(Iterator iter);
            void operator+=(Iterator iter);
            void operator-=(Iterator iter);
            bool operator==(Iterator iter);
            bool operator!=(Iterator iter);
    };
}

#endif