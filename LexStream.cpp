
#include "Lex.h"
#include "Allocator.h"
#include <cstring>
#include <fstream>

static const int INIT_MIN = 10;
static const int AVG_CHARS_PER_TOKEN = 3;
static const int GROWTH_FACTOR = 2;

using Lex::LexStream;
using Lex::Token;

static inline int max(int a, int b) {
    return a>b?a:b;
}

LexStream::LexStream(const int fileSize) {
    const int len = max(INIT_MIN, fileSize/AVG_CHARS_PER_TOKEN);
    stream = Global::getAllocator()->allocate<Token>(len);
    curr = stream;
    _end = stream + len;
}

LexStream::~LexStream() {
    delete [] stream;
}

Token* LexStream::allocate() {
    if(__builtin_expect(curr == _end, false)) {
        // allocate more
        const int size = _end-stream;
        Token* aux = Global::getAllocator()->allocate<Token>(GROWTH_FACTOR*size);
        std::memcpy(aux, stream, size*sizeof(Token));
        curr = aux+size;
        Global::getAllocator()->deallocate<Token>(stream);
        stream = aux;
        _end = aux + GROWTH_FACTOR*size;   
    }
    return curr++;
}

void LexStream::persist(const char* const file) {
    std::ofstream fout(file);
    Token* ptr = stream;
    while(ptr != curr) {
        fout << subtypeStrings[static_cast<int>(ptr->subType)] << '\n';
        ++ptr;
    }
    fout.flush();
    fout.close();
}

Lex::Token* LexStream::begin() {
    return stream;
}

Lex::Token* LexStream::end() {
    return curr;
}