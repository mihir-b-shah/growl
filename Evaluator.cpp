

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
};

static inline int convert(double w, int h, int j) {
    return static_cast<int>(w*(2*j+1)/(1 << h+1));
}

void Expr::print(const int width, std::ostream& out){
    Utils::SmallQueue<QueueItem,100> queue;
    QueueItem inp = {this,0,0};
    queue.push_back(inp);
    
    int currHeight = -1;
    int currJustif = 0;
    
    while(queue.size() > 0) {
        QueueItem obj = queue.front();
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
        
        
        
    }
}

static inline void bind(Token* tkn, Syntax::OpType* top){
    if(__builtin_expect(*top == Syntax::OpType::AMBIG_TYPE, false)){
        return;
    }
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

static inline void construct(Utils::Vector<Expr*>& output, SubType top){
    Expr* op1; Expr* op2;
    Op* ins;

    switch(Syntax::opType(top)){
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

Expr* Parse::parseExpr(Lex::Token* begin, Lex::Token* end) {
    Utils::SmallVector<Token*,100> stack;
    Utils::SmallVector<Expr*,200> output;
    
    for(auto tk = begin; tk!=end; ++tk) {
        switch(tk->subType){
            case SubType::OPAREN:
                stack.push_back(tk);
                break;
            case SubType::CPAREN:
            {
                bool parenFlg = false;
                while(stack.size() > 0 && (stack.eback()->subType != SubType::OPAREN)){
                    SubType th = stack.eback()->subType;
                    parenFlg = true;
                    stack.pop_back();
                    construct(output, th);
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
                if(tk == begin) {
                    myType = Syntax::OpType::UNARY;
                } else {
                    bind(tk-1, &myType);
                }
                bool assoc = Syntax::associate(me, myType) == Syntax::Assoc::LEFT;
                SubType top;
                // watch out for associative
                while(true){
                    // just easier control flow for me
                    if(stack.size() <= 0 || stack.eback()->type != Type::OPERATOR){
                        break;
                    }
                    top = stack.eback()->subType;
                    
                    Syntax::OpType topType = Syntax::opType(top);
                    const bool c1 = Syntax::precedence(me, myType) < Syntax::precedence(top, topType);
                    const bool c2 = Syntax::precedence(me, myType) == Syntax::precedence(top, topType) && assoc;

                    if(c1 || c2) {
                        stack.pop_back();
                        construct(output, top);
                    } else {
                        break;
                    }
                }
                stack.push_back(tk); 
                break;
            }
        }
    }
    while(stack.size() > 0) {
        SubType me = stack.eback()->subType;
        construct(output, me);
        stack.pop_back();
    }
    
    Expr* ret = output.eback();
    output.pop_back();
    return ret;
}