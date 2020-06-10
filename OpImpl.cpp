
#include "AST.h"
#include "Error.h"
#include "Syntax.h"

/**
argv is expected to be heap-allocated prior to the invocation
of this method. it will not be freed by this method either.
*/
Parse::Op::Op(Parse::FuncDef* def, int argc, Parse::Expr** argv){
    // throw error if the number of args give != argc(def).
    if(def->arity() != argc) {
        Global::specifyError("Function call args not match def.");
        throw Global::InvalidFunctionCall;
    }
    
    switch(argc) {
        case 1:
            Parse::Op::inputs.arg = *argv;
            break;
        case 2:
            Parse::Op::inputs.twoArgs.arg1 = *argv;
            Parse::Op::inputs.twoArgs.arg2 = *(argv+1);
            break;
        default:
            Parse::Op::inputs.args = argv;
            break;
    }
    
    Parse::Op::intrinsic = false;
    Parse::Op::driver.func = def;
}

// unary
Parse::Op::Op(Lex::SubType op, Parse::Expr* e1){
    Parse::Op::inputs.arg = e1;
    Parse::Op::intrinsic = true;
    switch(op) {
        case Lex::SubType::MINUS:
            Parse::Op::driver.intr = Parse::IntrOps::NEG;
            break;
        case Lex::SubType::ASTK:
            Parse::Op::driver.intr = Parse::IntrOps::DEREF;
            break;
        case Lex::SubType::NEG:
            Parse::Op::driver.intr = Parse::IntrOps::FLIP;
            break;
        case Lex::SubType::AMP:
            Parse::Op::driver.intr = Parse::IntrOps::ADDRESS;
            break;
        default:
            Global::specifyError("Invalid invocation of operator.");
            throw Global::InvalidOperatorInvocation;
    }
}    

// binary 
Parse::Op::Op(Lex::SubType op, Parse::Expr* e1, Parse::Expr* e2){
    Parse::Op::inputs.twoArgs.arg1 = e1;
    Parse::Op::inputs.twoArgs.arg2 = e2;
    Parse::Op::intrinsic = true;
    switch(op) {
        case Lex::SubType::PLUS:
            Parse::Op::driver.intr = Parse::IntrOps::ADD;
            break;
        case Lex::SubType::MINUS:
            Parse::Op::driver.intr = Parse::IntrOps::MINUS;
            break;
        case Lex::SubType::ASTK:
            Parse::Op::driver.intr = Parse::IntrOps::MULT;
            break;
        case Lex::SubType::DIV:
            Parse::Op::driver.intr = Parse::IntrOps::DIV;
            break;
        case Lex::SubType::MOD:
            Parse::Op::driver.intr = Parse::IntrOps::MOD;
            break;
        case Lex::SubType::DOT:
            Parse::Op::driver.intr = Parse::IntrOps::DOT;
            break;
        case Lex::SubType::GREATER:
            Parse::Op::driver.intr = Parse::IntrOps::GREATER;
            break;
        case Lex::SubType::LESS:
            Parse::Op::driver.intr = Parse::IntrOps::LESS;
            break;
        case Lex::SubType::EQUAL:
            Parse::Op::driver.intr = Parse::IntrOps::EQUAL;
            break;
        case Lex::SubType::AMP:
            Parse::Op::driver.intr = Parse::IntrOps::AND;
            break;
        case Lex::SubType::OR:
            Parse::Op::driver.intr = Parse::IntrOps::OR;
            break;
        case Lex::SubType::CARET:
            Parse::Op::driver.intr = Parse::IntrOps::XOR;
            break;
        case Lex::SubType::ASSN:
            Parse::Op::driver.intr = Parse::IntrOps::ASSN;
            break;
        case Lex::SubType::SHIFT:
            Parse::Op::driver.intr = Parse::IntrOps::SHIFT;
            break;
        default:
            Global::specifyError("Invalid invocation of operator.");
            throw Global::InvalidOperatorInvocation;
	}
}

Parse::Op::~Op(){
};

static inline Syntax::OpType detOpType(Parse::IntrOps type) {
    switch(type) {
        case Parse::IntrOps::NEG:
        case Parse::IntrOps::ADDRESS:
        case Parse::IntrOps::FLIP:
        case Parse::IntrOps::DEREF:
            return Syntax::OpType::UNARY;
        case Parse::IntrOps::ADD:
        case Parse::IntrOps::MINUS:
        case Parse::IntrOps::MULT:
        case Parse::IntrOps::DIV:
        case Parse::IntrOps::MOD:
        case Parse::IntrOps::DOT:
        case Parse::IntrOps::GREATER:
        case Parse::IntrOps::LESS:
        case Parse::IntrOps::EQUAL:
        case Parse::IntrOps::AND:
        case Parse::IntrOps::OR:
        case Parse::IntrOps::XOR:
        case Parse::IntrOps::ASSN:
        case Parse::IntrOps::SHIFT:
            return Syntax::OpType::BINARY;
        default:
            Global::specifyError("Invalid operator encountered.");
            throw Global::InvalidOperator;
    }
}

int Parse::Op::arity() const {
    if(Parse::Op::intrinsic) {
        return detOpType(Parse::Op::driver.intr);
    } else {
        return Parse::Op::driver.func->arity();
    }
}