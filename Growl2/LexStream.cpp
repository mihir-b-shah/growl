
#include <vector>
#include "Lex.h"
#include <cstring>

/* The class definition, for reference.

class LexStream {
    private:
        Lex::Token* stream;
        Lex::Token* curr;
        Lex::Token* end;
    public:
        LexStream();
        ~LexStream();
        Lex::Token* allocate();
}; 
*/

Lex::LexStream::LexStream(const int fileSize) {
    stream = new Lex::Token[fileSize/6];
    curr = stream;
    end = stream + fileSize/6;
}

Lex::LexStream::~LexStream() {
    delete [] stream;
}

Lex::Token* Lex::LexStream::allocate() {
    if(curr == end) {
        // allocate more
        Lex::Token* aux = new Lex::Token[2*(end-stream)];
        std::memcpy(aux, stream, (end-stream)*sizeof(Lex::Token));
        delete [] stream;
        curr = aux+(end-stream);
        stream = aux;
        end = stream + 2*(end-stream);
        return curr++;
    } else {
        return curr++;
    }
}