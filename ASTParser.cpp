
#include "AST.hpp"
#include "Parse.h"
#include "Lex.h"
#include "Error.h"
#include "Syntax.h"
#include <iostream>

using namespace Parse;

// Associated method with control node, bc forward decl.
AST* ControlNode::getSequentialBase(){
    return getBack()->at(getIdx()+1);
}

/** Handle errors for unimplemented features in the parser. */
static inline void unsupported(){
    Global::specifyError("Not supported yet.", __FILE__, __LINE__);
    throw Global::DeveloperError;
}

/** Main parser, parses between begin and end, which is offset number of tokens
 *  away from the program start. The calling control structure is cntrl */
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
            if(__builtin_expect(ret == nullptr, false)){
                // Specifically, this is bc constructors dont exist now.
				delete var;
                unsupported();
            }
            // construct the declaration and move on.
            Decl* decl = new Decl(var, const_cast<char*>(begin->pos));
            cntrl->seqAdd(decl);
            parse(offset + (ret-begin), ret, end, cntrl);
            break;
        }
        case Type::OPERATOR:
			// if the first token of expr is a binary operator this is synactically impossible.
            if(Syntax::opType(begin->subType) == Syntax::OpType::BINARY){
                Global::specifyError("Presence of binary operator invalidates expr.\n", 
						__FILE__, __LINE__);
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
			// call the expression parser.
            Expr* expr = parseExpr(begin, runner);
			// add the expression to the control sequence.
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
                    cntrl->seqAdd(loop);
					// handle the next control seq jump.
                    loop->setBackTrace(cntrl);
                    Lex::Token* next = parseLoop(offset, begin, loop); 
                    parse(offset + (next - begin), next, end, cntrl);
                    break;
                }
                case SubType::IF:
                {
                    Branch* br = new Branch();
                    cntrl->seqAdd(br);
					// handle the next control seq jump.
                    br->setBackTrace(cntrl);
                    Lex::Token* next = parseBranch(offset, begin, br); 
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

/** The method that actually gets called by the driver class */
void Parse::parseAST(int offset, Lex::Token* begin, Lex::Token* end, Control* cntrl){
    parse(offset, begin, end, cntrl);
}
