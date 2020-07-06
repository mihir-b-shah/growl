

#include <iostream>
#include <chrono>
#include <vector>
#include <cstdlib>
#include "Set.hpp"
#include <unordered_set>
#include "Benchmark.hpp"

using namespace std;

long long measureMySet(long long size){
    long long accm = 0;
    Utils::SmallSet<int,1000> mset;
    vector<int> inserts(size);
    srand(302993029);

    for(int i = 0; i<size; ++i){
        inserts[i] = rand();
        mset.insert(inserts[i]);
    }
    
    auto start = chrono::steady_clock::now();
    for(int i = 0; i<size; ++i){
        accm += mset.find(inserts[i]) == nullptr;
    }
    auto end = chrono::steady_clock::now();
    return (chrono::duration_cast<chrono::nanoseconds>(end - start).count())/(size/10);
}

long long measureStdSet(long long size){
    long long accm = 0;
    std::unordered_set<int> mset;
    vector<int> inserts(size);
    srand(302993029);

    for(int i = 0; i<size; ++i){
        inserts[i] = rand();
        mset.insert(inserts[i]);
    }

    auto start = chrono::steady_clock::now();
    for(int i = 0; i<size; ++i){
        accm += mset.find(inserts[i]) == mset.end();
    }
    auto end = chrono::steady_clock::now();

    return (chrono::duration_cast<chrono::nanoseconds>(end - start).count())/(size/10);
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
    //std::cout << "My set: " << measureMySet(argc) << "\n\n";
    //std::cout << "Std set: " << measureStdSet(argc) << '\n';
    
    size_t sizes[5] = {1000,10000,100000,1000000,10000000};
    const char* const descrs[2] = {"Std::set", "Utils::set"};
    long long (*ptrs[2])(long long) = {measureStdSet, measureMySet};
    
    Utils::ResultTable<80,5,2> table(sizes, descrs, ptrs);
    table.print(cout);
    
    return 0;
}