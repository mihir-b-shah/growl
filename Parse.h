
#ifndef PARSE_H
#define PARSE_H

#include "Lex.h"
#include "GroupFinder.hpp"

namespace Parse {    
    enum class SupportedType:char {_Expr, _Op, _Lit, _Var};

	class AST;
    class Expr;
    class Loop;
	class Variable;
    class Control;

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

	void parseAST(int offset, Lex::Token* begin, Lex::Token* end, Control* within);
    Expr* parseExpr(Lex::Token* begin, Lex::Token* end);
	Lex::Token* parseLoop(int offset, Lex::Token* begin, Loop* lp);
	Lex::Token* parseDecl(Lex::Token* begin, Variable* v, Control* within);
	GroupFinder* gf();

	class SymbolTable;
	SymbolTable* st();
	
}

#endif
