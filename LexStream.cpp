
#include "Lex.h"
#include "Global.hpp"
#include <cstring>
#include <fstream>

static const int INIT_MIN = 4;

static inline int max(int a, int b) {
    return a>b?a:b;
}

Lex::LexStream::LexStream(const int fileSize) {
    const int len = max(INIT_MIN, fileSize/6);
    Lex::LexStream::stream = Global::getAllocator()->allocate<Lex::Token>(len);
    Lex::LexStream::curr = stream;
    Lex::LexStream::end = stream + len;
}

Lex::LexStream::~LexStream() {
    // the heap will get unallocated at the end.
}

Lex::Token* Lex::LexStream::allocate() {
    if(__builtin_expect(curr == end, false)) {
        // allocate more
        const int size = Lex::LexStream::end-Lex::LexStream::stream;
        Lex::Token* aux = Global::getAllocator()->allocate<Lex::Token>(2*size);
        std::memcpy(aux, Lex::LexStream::stream, size*sizeof(Lex::Token));
        Lex::LexStream::curr = aux+size;
        Lex::LexStream::stream = aux;
        Lex::LexStream::end = aux + 2*size;   
    }
    return curr++;
}

void Lex::LexStream::persist(const char* const file) {
    std::ofstream fout(file);
    Lex::Token* ptr = Lex::LexStream::stream;
    while(ptr != Lex::LexStream::curr) {
        fout << subtypeStrings[ptr->subType] << '\n';
        ++ptr;
    }
    fout.flush();
    fout.close();
}