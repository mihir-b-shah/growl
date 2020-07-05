

#include <iostream>
#include <chrono>
#include <vector>
#include <cstdlib>
#include "Set.hpp"
#include <unordered_set>

using namespace std;

long long measureMySet(int seed){
    long long accm = 0;
    for(int size = 1000; size<100000000; size*=10){
        Utils::SmallSet<int,1000> mset;
        vector<int> inserts(size);
        srand(seed);

        for(int i = 0; i<size; ++i){
            inserts[i] = rand();
            mset.insert(inserts[i]);
        }
        
        cout << "Starting benchmark.\n";
        
        auto start = chrono::steady_clock::now();
        for(int i = 0; i<size; ++i){
            accm += mset.find(inserts[i]) == nullptr;
        }
        auto end = chrono::steady_clock::now();

        cout << "Elapsed time in nanoseconds for 10 finds, size " << size << ": "
            << (chrono::duration_cast<chrono::nanoseconds>(end - start).count())/(size/10) << '\n';
    }
	return accm;
}

long long measureStdSet(int seed){
    long long accm = 0;
    for(int size = 1000; size<100000000; size*=10){
        std::unordered_set<int> mset;
        vector<int> inserts(size);
        srand(seed);

        for(int i = 0; i<size; ++i){
            inserts[i] = rand();
            mset.insert(inserts[i]);
        }

        cout << "Starting benchmark.\n";
        
        auto start = chrono::steady_clock::now();
        for(int i = 0; i<size; ++i){
            accm += mset.find(inserts[i]) == mset.end();
        }
        auto end = chrono::steady_clock::now();

        cout << "Elapsed time in nanoseconds for 10 finds, size " << size << ": "
            << (chrono::duration_cast<chrono::nanoseconds>(end - start).count())/(size/10) << '\n';
    }
	return accm;
}

int main(int argc, char** argv)
{
    /*
    Utils::SmallSet<int,3> set;
    
    int inserts[9] = {3,495,1,15,35,27,80,19,39};

    for(int ins: inserts){
        // set.printSet(std::cout);
        set.insert(ins);
    }
    
    int erases[2] = {3,1};
    for(int er: erases){
        // set.printSet(std::cout);
        set.erase(er);
    }
    // set.printSet(std::cout);
    
    int checks[9] = {3,37,495,1,949,15,39,35,20};
    for(int check: checks){
        std::cout << (set.find(check)!=nullptr) << '\n';
    }
    
    return 0;
    */
    std::cout << "My set: " << measureMySet(argc) << "\n\n";
    std::cout << "Std set: " << measureStdSet(argc) << '\n';
    return 0;
}