
#ifndef AST_H
#define AST_H

#include "Lex.h"
#include "Syntax.h"
#include "Error.h"
#include <iostream>
#include <cstdlib>

namespace Parse {
        
    class AST {
    };

    class Expr;
    class ExprIterator {
        public:
            Expr* operator*();
            ExprIterator operator++();
            bool operator!=(ExprIterator iter);
    };
    
    class Expr : public AST {
        public:
            virtual int printRoot(char* buf) const;
            ExprIterator begin();
            ExprIterator end();
            void print(const int width, std::ostream& out);
    };

    class Control : public AST {
    };

    class FuncDef : public Control {
        private:
            int numArgs;
        public:
            int arity() const {
                Global::specifyError("Unimplemented function call.");
                throw Global::DeveloperError;
            }
    };

    class Branch : public Control {
    };

    /*
    later...
        class Switch : Control {
        };

        class Label : Control {
        };
    */

    class Loop : public Control {
    };

    enum class IntrOps {ADD, MINUS, NEG, MULT, DEREF, DIV, MOD, FLIP, DOT, GREATER, LESS, EQUAL, ADDRESS, AND, OR, XOR, ASSN, SHIFT};

    class Op;
    class OpIterator : ExprIterator {
        private:
            Op* handle;
            int pos;
        public:
            OpIterator(Op* hand, int p){
                handle = hand;
                pos = p;
            }
            Expr* operator*();
            OpIterator operator++();
            bool operator!=(OpIterator iter);
    };
    
    class Op : public Expr {
        friend class OpIterator;
        private:
            // small size optimization, avoid a heap allocation
            union {
                Expr* arg;
                Expr* twoArgs[2];
                Expr** args;
            } inputs;
            // tag the union
            bool intrinsic;
            union {
                FuncDef* func;
                IntrOps intr;
            } driver;
        public:
            Op(FuncDef* def, int argc, Expr** argv);
            Op(Lex::SubType op, Expr* e1);
            Op(Lex::SubType op, Expr* e1, Expr* e2);
            ~Op();
            int arity() const;
            int printRoot(char* buf) const;
            OpIterator begin();
            OpIterator end();
    };
    
    class Literal;
    class LitIterator : ExprIterator {
        public:
            LitIterator(){
            }
            Literal* operator*(){
                return nullptr;
            }
            LitIterator operator++(){
                Global::specifyError("LitIterator ++ called. should never.");
                throw Global::DeveloperError;
            }
            bool operator!=(LitIterator iter){
                return false;
            }
    };
    class Literal : public Expr {
        friend class LitIterator;
        private:
            enum {
                INT,
                FLOAT
            } type;
            union {
                long long intVal;
                long double fltVal;
            } value;
        public:
            Literal(){}
            ~Literal(){}
            long long getInt(){ return value.intVal;}
            long double getFlt(){ return value.fltVal;}
            bool isInt(){ return type == INT;}
            bool isFloat(){ return type == FLOAT;}
            void setInt(long long v){
                type = INT;
                value.intVal = v;
            }
            void setFlt(long double v){
                type = FLOAT;
                value.fltVal = v;
            }
            int printRoot(char* buf) const {
                switch(type){
                    case INT:
                        // print len 3.
                        return std::snprintf(buf,3,"%lld",value.intVal);
                    case FLOAT:
                        return std::snprintf(buf,3,"%lf",value.fltVal);
                    default:
                        Global::specifyError("Literal of invalid type.\n");
                        throw Global::DeveloperError;
                }
            }
            LitIterator begin(){
                return LitIterator();
            }
            LitIterator end(){
                return LitIterator();
            }
    };

    class Var : public Expr {
    };
}

#endif