
#ifndef PARSE_H
#define PARSE_H

#include "Lex.h"
#include "GroupFinder.hpp"

namespace Parse {    
    enum class SupportedType:char {_Expr, _Op, _Lit, _Var, _Decl, _Lp, _Br, _Ctl, _Seq, _Cast};

    class AST;
    class Expr;
    class Loop;
    class Variable;
    class Control;
    class Branch;

    enum class VarType:char {INT, LONG, CHAR, FLOAT, BOOL, VOID, OTHER};
    enum class IntrOps:char {ADD, MINUS, NEG, MULT, DEREF, DIV, MOD, FLIP, DOT,
            GREATER, LESS, EQUAL, ADDRESS, AND, OR, XOR, ASSN, LSHIFT, RSHIFT, CAST, OTHER};

    class ArgIterator {
        SupportedType type;
        AST* handle;
        int pos;
        
        public:
            ArgIterator(SupportedType mType, AST* mHandle);
            bool done();
            void next();
            AST* get();
    };

    void parseAST(int offset, Lex::Token* begin, Lex::Token* end, Control* within);
    Expr* parseExpr(Lex::Token* begin, Lex::Token* end);
    Lex::Token* parseLoop(int offset, Lex::Token* begin, Loop* lp);

    Lex::Token* parseBranch(int offset, Lex::Token* begin, Branch* br);

    Lex::Token* parseDecl(Lex::Token* begin, Variable* v, Control* within);
    GroupFinder* gf();

    class SymbolTable;
    SymbolTable* st();
}

#endif
