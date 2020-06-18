
#include "Lex.h"
#include "AST.h"
#include "Error.h"
#include "SmallVector.hpp"
#include "Syntax.h"

/**
 * Implement the shunting yard algorithm.
 * Idea borrowed in part from https://en.wikipedia.org/wiki/Shunting-yard_algorithm.
 */

using namespace Parse;
using namespace Lex;

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

static inline void construct(Utils::SmallVector<Expr*,100>& output, SubType top){
    Expr* op1, op2;
    switch(Syntax::opType(top)){
        case Syntax::OpType::UNARY:
            op1 = output.back();
            output.pop_back();
            Op* unary = new Op(top, op1);
            output.push_back(unary);
            break;
        case Syntax::OpType::BINARY:
            op1 = output.back();
            output.pop_back();
            op2 = output.back();
            output.pop_back();
            Op* binary = new Op(top, op1, op2);
            output.push_back(binary);
            break;
        default:
            Global::specifyError("Invalid operator 'arity'.");
            throw Global::InvalidExpression;
            
    }
}

Expr* parseExpr(Lex::Token* begin, Lex::Token* end) {
    Utils::SmallVector<Token*,100> stack;
    Utils::SmallVector<Expr*,200> output;
    
    for(auto tk = begin; tk!=end; ++tk) {
        switch(tk->subType){
            case SubType::OPAREN:
                stack.push_back(tk);
                break;
            case SubType::CPAREN:
                bool parenFlg = false;
                while(stack.size() > 0 && (stack.back()->subType != SubType::OPAREN)) {
                    SubType th = stack.back()->subType;
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
            case SubType::COLON:
            case SubType::OBRACK:
            case SubType::CBRACK:
            case SubType::COMMA:
            case SubType::SEMICOLON:
                Global::specifyError("Invalid group token in expression.");
                throw Global::InvalidExpression;
            case SubType::INT_LITERAL:
            case SubType::CHAR_LITERAL:
                Literal* intlit = Global::getAllocator()->allocate<Literal>(1);
                intlit->type = Literal::INT;
                intlit->value.intVal = v;
                output.push_back(intlit);
                break;
            case SubType::FLT_LITERAL:
                Literal* fltlit = Global::getAllocator()->allocate<Literal>(1);
                intlit->type = Literal::FLOAT;
                intlit->value.intVal = v;
                output.push_back(fltlit);
                break;
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
                SubType me = tk->subType;
                Syntax::OpType myType = Syntax::opType(me);
                bool assoc = Syntax::assoc(myType, me) == Syntax::Assoc::LEFT;
                SubType top;
                // watch out for associative
                while(true){
                    // just easier control flow for me
                    if(stack.size() <= 0 || stack.back()->type != Type::OPERATOR){
                        break;
                    }
                    top = stack.back()->subType;
                    Syntax::OpType topType = Syntax::opType(top);
                    const bool c1 = precedence(me, myType) > precedence(top, topType));
                    const bool c2 = precedence(me, myType) == precedence(top, topType) && assoc;
                    if(c1 || c2) {
                        stack.pop_back();
                        construct(output, top);
                    }
                }
                stack.push(tk); 
                break;
        }
    }
    while(stack.size() > 0) {
        SubType me = stack.back()->subType;
        construct(output, me);
    }
    assert(output.size() == 1);
    Expr* ret = output.back();
    output.pop_back();
    return ret;
}