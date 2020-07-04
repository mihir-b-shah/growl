
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

    class Expr : public AST {
        friend class ArgIterator;
        public:
            virtual int printRoot(char* buf) const = 0;
            virtual ArgIterator iterator() = 0;
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

    enum class IntrOps:char {ADD, MINUS, NEG, MULT, DEREF, DIV, MOD, FLIP, DOT, GREATER, LESS, EQUAL, ADDRESS, AND, OR, XOR, ASSN, SHIFT};
    
    class Op : public Expr {
        friend class ArgIterator;
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
            ArgIterator iterator();
    };

    class Literal : public Expr {
        friend class ArgIterator;
        private:
            enum:char {
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
                        return std::snprintf(buf,4,"%lld",value.intVal);
                    case FLOAT:
                        return std::snprintf(buf,4,"%lf",value.fltVal);
                    default:
                        Global::specifyError("Literal of invalid type.\n");
                        throw Global::DeveloperError;
                }
            }
            Parse::ArgIterator iterator(){
                return ArgIterator(SupportedType::_Lit, this, 0);
            }
    };

    enum class VarType:char {INT, LONG, CHAR, FLOAT, BOOL, VOID};

    class Variable : public Expr {
        friend class ArgIterator;
        
        typedef unsigned char byte;
        
        const char* name;
        byte len;
        VarType type;
        byte ptrLvl;
        bool _unsigned; // true if so.

        public:
            Variable(const char* _name, char _len, Lex::SubType _type, char _ptrLvl);
            ~Variable();
            int printRoot(char* buf) const override;
            Parse::ArgIterator iterator() override;
    };
}

#endif
