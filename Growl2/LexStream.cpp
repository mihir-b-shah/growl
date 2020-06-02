
#include <vector>
#include "Lex.h"
#include <cstring>
#include <fstream>

static const int INIT_MIN = 4;

static inline int max(int a, int b) {
    return a>b?a:b;
}

Lex::LexStream::LexStream(const int fileSize) {
    const int len = max(INIT_MIN, fileSize/6);
    stream = new Lex::Token[len];
    curr = stream;
    end = stream + len;
}

Lex::LexStream::~LexStream() {
    delete [] stream;
}

Lex::Token* Lex::LexStream::allocate() {
    if(__builtin_expect(curr == end, false)) {
        // allocate more
        const int size = end-stream;
        Lex::Token* aux = new Lex::Token[2*size];
        std::memcpy(aux, stream, size*sizeof(Lex::Token));
        delete [] stream;
        curr = aux+size;
        stream = aux;
        end = aux + 2*size;   
    }
    return curr++;
}

void Lex::LexStream::persist(const char* const file) {
    std::ofstream fout(file);
    Lex::Token* ptr = stream;
    while(ptr != curr) {
        fout << subtypeStrings[ptr->subType] << '\n';
        ++ptr;
    }
    fout.flush();
    fout.close();
}