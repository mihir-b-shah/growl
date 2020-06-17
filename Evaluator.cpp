
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
                    parenFlg = true;
                    // TO BE IMPL: construct(output, (Operator) stack.pop().subType);
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
                        // TO IMPL construct(output, top);
                    }
                }
                stack.push(tk); 
                break;
        }
    }
}
 
/*
// for now, only const expr
Expression parseExpr(int start, int end) {
    // stack can only contain groups and operators
    Stack<Lexeme> stack = new Stack<>();
    Stack<Expression> output = new Stack<>();
    
    // need to add functions
    // https://en.wikipedia.org/wiki/Shunting-yard_algorithm
    for(int ptr = start; ptr<end; ++ptr) {
        Lexeme lexeme = lexemes.get(ptr);
        switch(lexeme.type) {
            case LITERAL:
                Literal lit = new Literal(lexeme);
                output.push(lit);
                break;
            case OPERATOR:
                Operator me = (Operator) lexeme.subType;
                Operator top;
                // watch out for associative
                while(stack.size() > 0 && stack.peek().type == LexType.OPERATOR 
                        && ((top = (Operator) stack.peek().subType).precedence() 
                        > me.precedence() || top.precedence() == me.precedence() 
                        && me.associate() == Associativity.LEFT_TO_RIGHT)) {
                    stack.pop();
                    construct(output, top);
                }
                stack.push(lexeme); 
                break;
            case GROUP:
                switch((Group) lexeme.subType) {
                    case OPEN_PAREN:
                        stack.push(lexeme);
                        break;
                    case CLOSE_PAREN:
                        boolean flg = false;
                        while(stack.size() > 0 && (stack.peek().type != LexType.GROUP
                                || ((Group) stack.peek().subType) != Group.OPEN_PAREN)) {
                            flg = true;
                            construct(output, (Operator) stack.pop().subType);
                        }
                        // only case, if hit paren
                        if(stack.size() > 0) {
                            stack.pop();
                        }
                        if(!flg) {
                            throw new ParseError();
                        }
                        break;
                    default:
                        throw new ParseError();
                }
        }
    }
    while(stack.size() > 0) {
        construct(output, (Operator) stack.pop().subType);
    }
    assert(output.size() == 1);
    return output.pop();
}
*/