
#include "Lex.h"
#include "AST.h"
#include "Error.h"
#include "Syntax.h"

using namespace Parse;
using Lex::SubType;

/**
argv is expected to be heap-allocated prior to the invocation
of this method. it will not be freed by this method either.
*/
Op::Op(FuncDef* def, int argc, Expr** argv){
    // throw error if the number of args give != argc(def).
    if(def->arity() != argc) {
        Global::specifyError("Function call args not match def.");
        throw Global::InvalidFunctionCall;
    }
    
    switch(argc) {
        case 1:
            Op::inputs.arg = *argv;
            break;
        case 2:
            inputs.twoArgs.arg1 = *argv;
            inputs.twoArgs.arg2 = *(argv+1);
            break;
        default:
            inputs.args = argv;
            break;
    }
    
    Op::intrinsic = false;
    Op::driver.func = def;
}

// unary
Op::Op(SubType op, Expr* e1){
    Op::inputs.arg = e1;
    Op::intrinsic = true;
    switch(op) {
        case SubType::MINUS:
            driver.intr = NEG;
            break;
        case SubType::ASTK:
            driver.intr = DEREF;
            break;
        case SubType::NEG:
            driver.intr = FLIP;
            break;
        case SubType::AMP:
            driver.intr = ADDRESS;
            break;
        default:
            Global::specifyError("Invalid invocation of operator.");
            throw Global::InvalidOperatorInvocation;
    }
}    

// binary 
Op::Op(SubType op, Expr* e1, Expr* e2){
    Op::inputs.twoArgs.arg1 = e1;
    inputs.twoArgs.arg2 = e2;
    Op::intrinsic = true;
    switch(op) {
        case SubType::PLUS:
            driver.intr = ADD;
            break;
        case SubType::MINUS:
            driver.intr = MINUS;
            break;
        case SubType::ASTK:
            driver.intr = MULT;
            break;
        case SubType::DIV:
            driver.intr = DIV;
            break;
        case SubType::MOD:
            driver.intr = MOD;
            break;
        case SubType::DOT:
            driver.intr = DOT;
            break;
        case SubType::GREATER:
            driver.intr = GREATER;
            break;
        case SubType::LESS:
            driver.intr = LESS;
            break;
        case SubType::EQUAL:
            driver.intr = EQUAL;
            break;
        case SubType::AMP:
            driver.intr = AND;
            break;
        case SubType::OR:
            driver.intr = OR;
            break;
        case SubType::CARET:
            driver.intr = XOR;
            break;
        case SubType::ASSN:
            driver.intr = ASSN;
            break;
        case SubType::SHIFT:
            driver.intr = SHIFT;
            break;
        default:
            Global::specifyError("Invalid invocation of operator.");
            throw Global::InvalidOperatorInvocation;
    }
}

Op::~Op(){
};

static inline Syntax::OpType detOpType(IntrOps type) {
    switch(type) {
        case NEG:
        case ADDRESS:
        case FLIP:
        case DEREF:
            return Syntax::OpType::UNARY;
        case ADD:
        case MINUS:
        case MULT:
        case DIV:
        case MOD:
        case DOT:
        case GREATER:
        case LESS:
        case EQUAL:
        case AND:
        case OR:
        case XOR:
        case ASSN:
        case SHIFT:
            return Syntax::OpType::BINARY;
        default:
            Global::specifyError("Invalid operator encountered.");
            throw Global::InvalidOperator;
    }
}

int Op::arity() const {
    if(Op::intrinsic) {
        return detOpType(Op::driver.intr);
    } else {
        return driver.func->arity();
    }
}