
#ifndef AST_H
#define AST_H

#include "Lex.h"
#include "Parse.h"
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
            virtual Expr* get() = 0;
			// std does not allow covariant value, only reference
            virtual ExprIterator* nextArg() = 0;
            virtual bool done() = 0;
    };
    
    class Expr : public AST {
        public:
            virtual int printRoot(char* buf) const = 0;
            virtual ExprIterator* iterator() = 0;
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
    class OpIterator : public ExprIterator {
        private:
            Op* handle;
            int pos;
        public:
            OpIterator(Op* hand, int p){
                handle = hand;
                pos = p;
            }
            Expr* get();
            OpIterator* nextArg();
            bool done();
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
			/* code is wrong. constructor should accept for unary/binary
			   the small vector optimization */
            Op(FuncDef* def, int argc, Expr** argv);
			Op(FuncDef* def, Expr* e1);
			Op(FuncDef* def, Expr* e1, Expr* e2);
            Op(Lex::SubType op, Expr* e1);
            Op(Lex::SubType op, Expr* e1, Expr* e2);
            ~Op();
            int arity() const;
            int printRoot(char* buf) const;
            OpIterator* iterator();
    };
    
    class Literal;
    class LitIterator : public ExprIterator {
        public:
            LitIterator(){
            }
            Expr* get(){
                return nullptr;
            }
            LitIterator* nextArg(){
                Global::specifyError("LitIterator ++ called. should never.");
                throw Global::DeveloperError;
            }
            bool done(){
                return true;
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
				std::cout << "int lit gen called. " << v << '\n';
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
                        return std::snprintf(buf,4,"%lld",value.intVal);
                    case FLOAT:
                        return std::snprintf(buf,4,"%lf",value.fltVal);
                    default:
                        Global::specifyError("Literal of invalid type.\n");
                        throw Global::DeveloperError;
                }
            }
			// i am crying
			// heap allocating for a useless placeholder object
            LitIterator* iterator(){
				//std::cout << "lit iterator.\n";
				return new LitIterator();
            }
    };

    class Var : public Expr {
    };
}

#endif
