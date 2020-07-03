
#include "Lex.h"
#include "AST.hpp"
#include "Error.h"
#include "Vector.hpp"
#include "Syntax.h"
#include "Parse.h"
#include "Queue.hpp"
#include <cassert>
#include <iostream>

/**
 * Implement the shunting yard algorithm.
 * Idea borrowed in part from https://en.wikipedia.org/wiki/Shunting-yard_algorithm.
 */

using namespace Parse;
using namespace Lex;

struct QueueItem {
    Expr* bt;
    int height;
    int xJust;
	
	QueueItem(){
	}
	
	QueueItem(Expr* b, int h, int x){
		bt = b;
		height = h;
		xJust = x;
	}
};

static inline int convert(double w, int h, int j) {
    return static_cast<int>(w*(2*j+1)/(1 << h+1));
}

void Expr::print(const int width, std::ostream& out){
	out << '\n';
	for(int i = 0; i<width; ++i){
		out << '_';
	}
	out << '\n';
	
    Utils::SmallQueue<QueueItem,100> queue;
    QueueItem inp = {this,0,0};
    queue.push_back(inp);
    
    int currHeight = -1;
    int currJustif = 0;
    
    while(queue.size() > 0) {
        QueueItem obj = queue.front();
		//std::cerr << "xJust: " << obj.xJust << "h: " << obj.height << '\n';
        queue.pop_front();
        
        if(currHeight != obj.height){
            out << '\n';
            currHeight = obj.height;
            currJustif = 0;
        }
        
        char buf[4] = {'\0'};
        int count = obj.bt->printRoot(buf)/2;
        int amt = convert(width, obj.height, obj.xJust);
        for(int i = 0; i<amt-currJustif-count; ++i){
            out << ' ';
        }
        out << buf;
        currJustif = amt;
        
		int ctr = 0;
		auto iter = obj.bt->iterator();
		
		while(!(iter.done())){
			queue.push_back(QueueItem(static_cast<Op*>(iter.get()), 1+obj.height, 2*obj.xJust+ctr));
			iter.next();
			++ctr;
		}
    }
	out << '\n';
	for(int i = 0; i<width; ++i){
		out << '_';
	}
	out << '\n';
}

static inline void bind(Token* tkn, Syntax::OpType* top){
    // std::cout << "Before\n";
	if(__builtin_expect(*top != Syntax::OpType::AMBIG_TYPE, true)){
        return;
    }
	// std::cout << "After\n";
    switch(tkn->type){
        case Type::LITERAL:
        case Type::ID: // ids are useless for now...
            *top = Syntax::OpType::BINARY;
            break;
        case Type::OPERATOR:
            *top = Syntax::OpType::UNARY;
            break;
        case Type::GROUP:
            // yeeeeee very fast jump tables. i like jump tables.... 
            *top = tkn->subType == SubType::OPAREN ? Syntax::OpType::UNARY : Syntax::OpType::BINARY;
            break;
        default:
            Global::specifyError("Invalid token in expression.");
            throw Global::InvalidExpression;
    }
}

static inline void construct(Utils::Vector<Expr*>& output, SubType top, Syntax::OpType det){
    Expr* op1; Expr* op2;
    Op* ins;

	// at some point, change the 'new' to Global->alloc. Maybe pass void** as the params.
    switch(det){
        case Syntax::OpType::UNARY:
            op1 = output.eback();
            output.pop_back();
            ins = new Op(top, op1);
            output.push_back(ins);
            break;
        case Syntax::OpType::BINARY:
            op1 = output.eback();
            output.pop_back();
            op2 = output.eback();
            output.pop_back();
            ins = new Op(top, op2, op1);
            output.push_back(ins);
            break;
        default:
            Global::specifyError("Invalid operator 'arity'.");
            throw Global::InvalidExpression;
            
    }
}

struct BoundToken {
	Token* tkn;
	Syntax::OpType type;
	
	// for the use of containers in default construction.
	BoundToken(){
	}
	
	BoundToken(Token* t){
		tkn = t;
		type = Syntax::OpType::AMBIG_TYPE;
	}
	
	BoundToken(Token* t, Syntax::OpType ot){
		tkn = t;
		type = ot;
	}
};

Expr* Parse::parseExpr(Lex::Token* begin, Lex::Token* end) {
    Utils::SmallVector<BoundToken,100> stack;
    Utils::SmallVector<Expr*,200> output;
    
    for(auto tk = begin; tk!=end; ++tk) {
        switch(tk->subType){
            case SubType::OPAREN:
                stack.push_back(BoundToken(tk));
                break;
            case SubType::CPAREN:
            {
                bool parenFlg = false;
                while(stack.size() > 0 && (stack.eback().tkn->subType != SubType::OPAREN)){
                    SubType th = stack.eback().tkn->subType;
					Syntax::OpType type = stack.eback().type;
                    parenFlg = true;
                    stack.pop_back();
                    construct(output, th, type);
                }
                // only case, if hit paren
                if(stack.size() > 0) {
                    stack.pop_back();
                }
                if(!parenFlg) {
                    Global::specifyError("Parenthesis possibly not balanced.");
                    throw Global::InvalidExpression;
                }
                break;
            }
            case SubType::COLON:
            case SubType::OBRACK:
            case SubType::CBRACK:
            case SubType::COMMA:
            case SubType::SEMICOLON:
                Global::specifyError("Invalid group token in expression.");
                throw Global::InvalidExpression;
            case SubType::INT_LITERAL:
            case SubType::CHAR_LITERAL:
            {
                Literal* intlit = Global::getAllocator()->allocate<Literal>(1);
                intlit->setInt(tk->value.holder.ival);
                output.push_back(intlit);
                break;
            }
            case SubType::FLT_LITERAL:
            {
                Literal* fltlit = Global::getAllocator()->allocate<Literal>(1);
                fltlit->setFlt(tk->value.holder.fval);
                output.push_back(fltlit);
                break;
            }
            case SubType::PLUS:
            case SubType::MINUS:
            case SubType::ASTK:
            case SubType::DIV:
            case SubType::MOD:
            case SubType::NEG:
            case SubType::DOT:
            case SubType::GREATER:
            case SubType::LESS:
            case SubType::EQUAL:
            case SubType::AMP:
            case SubType::OR:
            case SubType::CARET:
            case SubType::ASSN:
            case SubType::SHIFT:
            {
                SubType me = tk->subType;
                Syntax::OpType myType = Syntax::opType(me);
                if(tk == begin && myType != Syntax::OpType::BINARY) {
                    myType = Syntax::OpType::UNARY;
                } else {
                    bind(tk-1, &myType);
                }
				
                bool assoc = Syntax::associate(me, myType) == Syntax::Assoc::LEFT;
                SubType top;
                // watch out for associative
                while(true){
                    // just easier control flow for me
                    if(stack.size() <= 0 || stack.eback().tkn->type != Type::OPERATOR){
                        break;
                    }
                    top = stack.eback().tkn->subType;
                    
                    Syntax::OpType topType = stack.eback().type;
                    const bool c1 = Syntax::precedence(me, myType) < Syntax::precedence(top, topType);
                    const bool c2 = Syntax::precedence(me, myType) == Syntax::precedence(top, topType) && assoc;

                    if(c1 || c2) {
                        stack.pop_back();
                        construct(output, top, topType);
                    } else {
                        break;
                    }
                }
                stack.push_back(BoundToken(tk, myType)); 
                break;
            }
        }
    }
    while(stack.size() > 0) {
        SubType me = stack.eback().tkn->subType;
        construct(output, me, stack.eback().type);
        stack.pop_back();
    }
    
    Expr* ret = output.eback();
    output.pop_back();
    return ret;
}
