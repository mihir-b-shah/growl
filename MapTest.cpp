

#include <iostream>
/*#include <chrono>
#include <unordered_set>
#include <vector>
#include <cstdlib> */
#include "Set.hpp"

//using namespace std;

struct MyTraits{
    static int emptyVal(){return 0;}
    static int tombstoneVal(){return -1;}
    static size_t hash(const int x){return x;}
    static bool equal(const int x, const int y){return x==y;}
};

int main(int argc, char** argv)
{
    Utils::SmallSet<int,20,MyTraits> set;
    std::cout << "hi.\n";
    return 0;
    /*
    for(int size = 1000; size<100000000; size*=10){
        unordered_set<int> mset;
        vector<int> inserts(size);
        srand(argc);

        for(int i = 0; i<size; ++i){
            inserts[i] = rand();
            mset.insert(inserts[i]);
        }
        
        auto start = chrono::steady_clock::now();

        for(int i = 0; i<size; ++i){
            mset.find(inserts[i]);
        }

        auto end = chrono::steady_clock::now();

        cout << "Elapsed time in nanoseconds for size " << size << ": "
            << (chrono::duration_cast<chrono::nanoseconds>(end - start).count())/size << '\n';
    }
	return 0;
    */
    
}