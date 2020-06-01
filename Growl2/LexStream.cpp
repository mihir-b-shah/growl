
#include <vector>
#include "Lex.h"
#include <cstring>
#include <fstream>

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

static const char* const table[] = {"IF", "ELSE", "GOTO", "RETURN", 
"WHILE", "SWITCH", "CASE", "DEFAULT", "BREAK", "UNSIGNED", "INT", "LONG", 
"CHAR", "FLOAT", "BOOL", "VOID", "OPAREN", "CPAREN", "COLON", "OBRACK", 
"CBRACK", "COMMA", "SEMICOLON", "INT_LITERAL", "FLT_LITERAL", "CHAR_LITERAL", 
"PLUS", "MINUS", "ASTK", "DIV", "MOD", "NEG", "DOT", "GREATER", "LESS", "AMP", 
"OR", "CARET", "ASSN"};


void Lex::LexStream::persist(const char* const file) {
    std::ofstream fout(file);
    Lex::Token* ptr = stream;
    while(ptr != end) {
        fout << stream->subType << '\n';
        ++ptr;
    }
    fout.flush();
    fout.close();
}