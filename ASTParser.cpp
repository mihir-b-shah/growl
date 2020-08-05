
#include "AST.hpp"
#include "Parse.h"
#include "Lex.h"
#include "Error.h"
#include "Syntax.h"
#include <iostream>

using namespace Parse;

static inline void unsupported(){
    Global::specifyError("Not supported yet.", __FILE__, __LINE__);
    throw Global::DeveloperError;
}

static void parse(int offset, Lex::Token* begin, Lex::Token* end, Control* cntrl){
    using Lex::SubType;
    using Lex::Type;

    if(begin >= end) return;

    switch(begin->type){
        case Type::DATATYPE:
        {
            Variable* var = new Variable();
            // guaranteed that semicolon is found.
            Lex::Token* ret = parseDecl(begin, var, cntrl);
            // POTENTIAL FOR STACK OVERFLOW IF THIS
            // RECURSION GOES TOO DEEP
            if(__builtin_expect(ret == nullptr, false)){
                // Specifically, this is bc constructors dont exist now.
                unsupported();
            }
            // ill optimize these allocations later.
            Decl* decl = new Decl(var, const_cast<char*>(begin->pos));
            cntrl->seqAdd(decl);
            parse(offset + (ret-begin), ret, end, cntrl);
            break;
        }
        case Type::OPERATOR:
            if(Syntax::opType(begin->subType) == Syntax::OpType::BINARY){
                Global::specifyError(
                                "Presence of binary operator invalidates expr.\n");
                throw Global::InvalidExpression;
            }
        case Type::ID:
        case Type::LITERAL:
        {
            Lex::Token* runner = begin;
            while(runner != end && runner->subType != SubType::SEMICOLON){
                ++runner;
            }    
            if(begin != end && runner == end){
                Global::specifyError("Semicolon not found.\n", __FILE__, __LINE__);
                throw Global::InvalidExpression;
            }
            Expr* expr = parseExpr(begin, runner);
            expr->print(80, std::cout);
            cntrl->seqAdd(expr);
            parse(offset + (runner-begin) + 1, runner + 1, end, cntrl);    
            break;
        }
        case Type::CONTROL:
        {
            switch(begin->subType){
                case SubType::WHILE:
                {
                    Loop* loop = new Loop();
                    Lex::Token* next = parseLoop(offset, begin, loop); 
                    cntrl->seqAdd(loop);
                    parse(offset + (next - begin), next, end, cntrl);
                    break;
                }
                case SubType::IF:
                {
                    Branch* br = new Branch();
                    Lex::Token* next = parseBranch(offset, begin, br); 
                    cntrl->seqAdd(br);
                    parse(offset + (next - begin), next, end, cntrl);
                    break;
                }
                default:
                    unsupported();
                    break;
            }
            break;
        }
        case Type::GROUP:
            if(__builtin_expect(begin->subType != SubType::CBRACK, false)){
                unsupported();
            }
            break;
    }
}

/** Stack overflow warning */
void Parse::parseAST(int offset, Lex::Token* begin, Lex::Token* end, Control* cntrl){
    parse(offset, begin, end, cntrl);
}
