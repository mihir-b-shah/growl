
#ifndef PARSE_H
#define PARSE_H

#include "Lex.h"

namespace Parse {    
    enum class SupportedType {_Expr, _Op, _Lit};

    class Expr;
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
}

#endif
