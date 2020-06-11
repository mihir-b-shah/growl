
#ifndef AST_H
#define AST_H

#include "Lex.h"
#include "Syntax.h"
#include "Error.h"

namespace Parse {
    class AST {
    };

    class Expr : public AST {
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

    enum IntrOps {ADD, MINUS, NEG, MULT, DEREF, DIV, MOD, FLIP, DOT, GREATER, LESS, EQUAL, ADDRESS, AND, OR, XOR, ASSN, SHIFT};

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
    };

    class Literal : public Expr {
    };

    class Var : public Expr {
    };
}

#endif