
#include "Lex.h"
#include "AST.hpp"
#include "Error.h"
#include "Syntax.h"
#include <cstdlib>

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
            driver.intr = IntrOps::NEG;
            break;
        case SubType::ASTK:
            driver.intr = IntrOps::DEREF;
            break;
        case SubType::NEG:
            driver.intr = IntrOps::FLIP;
            break;
        case SubType::AMP:
            driver.intr = IntrOps::ADDRESS;
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
            driver.intr = IntrOps::ADD;
            break;
        case SubType::MINUS:
            driver.intr = IntrOps::MINUS;
            break;
        case SubType::ASTK:
            driver.intr = IntrOps::MULT;
            break;
        case SubType::DIV:
            driver.intr = IntrOps::DIV;
            break;
        case SubType::MOD:
            driver.intr = IntrOps::MOD;
            break;
        case SubType::DOT:
            driver.intr = IntrOps::DOT;
            break;
        case SubType::GREATER:
            driver.intr = IntrOps::GREATER;
            break;
        case SubType::LESS:
            driver.intr = IntrOps::LESS;
            break;
        case SubType::EQUAL:
            driver.intr = IntrOps::EQUAL;
            break;
        case SubType::AMP:
            driver.intr = IntrOps::AND;
            break;
        case SubType::OR:
            driver.intr = IntrOps::OR;
            break;
        case SubType::CARET:
            driver.intr = IntrOps::XOR;
            break;
        case SubType::ASSN:
            driver.intr = IntrOps::ASSN;
            break;
        case SubType::SHIFT:
            driver.intr = IntrOps::SHIFT;
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
        case IntrOps::NEG:
        case IntrOps::ADDRESS:
        case IntrOps::FLIP:
        case IntrOps::DEREF:
            return Syntax::OpType::UNARY;
        case IntrOps::ADD:
        case IntrOps::MINUS:
        case IntrOps::MULT:
        case IntrOps::DIV:
        case IntrOps::MOD:
        case IntrOps::DOT:
        case IntrOps::GREATER:
        case IntrOps::LESS:
        case IntrOps::EQUAL:
        case IntrOps::AND:
        case IntrOps::OR:
        case IntrOps::XOR:
        case IntrOps::ASSN:
        case IntrOps::SHIFT:
            return Syntax::OpType::BINARY;
        default:
            Global::specifyError("Invalid operator encountered.");
            throw Global::InvalidOperator;
    }
}

int Op::arity() const {
    if(Op::intrinsic) {
        return static_cast<int>(detOpType(Op::driver.intr));
    } else {
        return driver.func->arity();
    }
}

static const char* opDisplay[] = {"+","-","-u","*","*u","/","%","~",".",">","<","<","==","&u","&","|","^","=","<<"};

// prints max of 3 chars.
int Op::printRoot(char* buf) const {
    if(Op::intrinsic) {
        return std::snprintf(buf,3,"%3s",opDisplay[static_cast<int>(Op::driver.intr)]);
    } else {
        return std::snprintf(buf,3,"%3s","FUN");
    }
    
}