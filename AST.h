
#ifdef AST_H
#define AST_H

namespace Parse {
    class AST {

    };

    class Expr : public AST {
        
    };

    class Control : public AST {

    };

    class FuncDef : public Control {

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

    enum IntrOps {"ADD", "MINUS", "NEG", "MULT", "DEREFERENCE", "DIV", "MOD", "NEG", 
                  "DOT", "GREATER", "LESS", "EQUAL", "ADDRESS", "AND", "OR", "CARET", 
                  "ASSN", "SHIFT"};

    class Op : public Expr {
        int opCt;
        Expr* inputs;
        union {
            FuncDef* func;
            IntrFunc* intr; 
        } driver;
    };

    class Literal : public Expr {

    };

    class Var : public Expr {

    };
}

#endif