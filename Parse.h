
#ifndef PARSE_H
#define PARSE_H

#include "Lex.h"
#include "GroupFinder.hpp"

namespace Parse {    
    enum class SupportedType:char {_Expr, _Op, _Lit, _Var};

    class Expr;
    class Loop;
    
    class ArgIterator {
        SupportedType type;
        Expr* handle;
        
        union {
            int pos;
            void* obj;
        } aux;
        
        public:
            ArgIterator(SupportedType mType, Expr* mHandle, void* pObj);
            ArgIterator(SupportedType mType, Expr* mHandle, int pos);
            bool done();
            void next();
            Expr* get();
    };

    Expr* parseExpr(Lex::Token* begin, Lex::Token* end);
    Loop* parseLoop(int offset, Lex::Token* begin, Lex::Token* end);
	GroupFinder* gf();
}

#endif
