
#include "Lex.h"
#include "Allocator.h"
#include <cstring>
#include <fstream>

static const int INIT_MIN = 10;
static const int AVG_CHARS_PER_TOKEN = 3;
static const int GROWTH_FACTOR = 2;

static inline int max(int a, int b) {
    return a>b?a:b;
}

Lex::LexStream::LexStream(const int fileSize) {
    const int len = max(INIT_MIN, fileSize/AVG_CHARS_PER_TOKEN);
    Lex::LexStream::stream = new Lex::Token[len];
    Lex::LexStream::curr = stream;
    Lex::LexStream::end = stream + len;
}

Lex::LexStream::~LexStream() {
    delete [] stream;
}

Lex::Token* Lex::LexStream::allocate() {
    if(__builtin_expect(curr == end, false)) {
        // allocate more
        const int size = Lex::LexStream::end-Lex::LexStream::stream;
        Lex::Token* aux = new Lex::Token[GROWTH_FACTOR*size];
        std::memcpy(aux, Lex::LexStream::stream, size*sizeof(Lex::Token));
        Lex::LexStream::curr = aux+size;
        delete [] stream;
        Lex::LexStream::stream = aux;
        Lex::LexStream::end = aux + GROWTH_FACTOR*size;   
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