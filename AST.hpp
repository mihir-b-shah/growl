
#ifndef AST_H
#define AST_H

#include "Lex.h"
#include "Syntax.h"
#include "Error.h"
#include <iostream>
#include <cstdlib>
#include "Iterator.hpp"

namespace Parse {
        
    class AST {
    };

    class ExprIterator : public Iterator<Expr> {
        friend class Expr;
        private:
            ExprIterator(Expr* ptr){
                curr = ptr;
            }
        public:
            Expr* operator*(){
                return curr;
            }
            ExprIterator operator++(){
                return *this;
            }
            ExprIterator operator+(ExprIterator iter){
                return *this;
            }
            ExprIterator operator-(ExprIterator iter){
                return *this;
            }
            void operator+=(ExprIterator iter){
                
            }
            void operator-=(ExprIterator iter){
                
            }
            bool operator==(ExprIterator iter){
                return true;
            }
            bool operator!=(ExprIterator iter){
                return true;
            }
    };
    
    class Expr : public AST {
        public:
            virtual int printRoot(char* buf) const;
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

    class Op : public Expr {
        private:
            // small size optimization, avoid a heap allocation
            union {
                Expr* arg;
                struct {
                    Expr* arg1;
                    Expr* arg2;
                } twoArgs;
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
    };

    class Literal : public Expr {
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
    };

    class Var : public Expr {
    };
}

#endif