
#include "AST.hpp"
#include "Vector.hpp"
#include "CodeGen.hpp"
#include <utility>

using namespace CodeGen;

/* Never call nextLabel directly, call this instead. */
static std::pair<Label,bool> getLabel(Parse::AST* ast){
    Label brOut;
    if(CodeGen::getFromAST(ast->getHash()) == Label::nullLabel()){
        // doesnt exist right now.
        insertASTLbl(ast->getHash(), brOut = nextLabel());
        return {brOut, true};
    } else {
        brOut = getFromAST(ast->getHash());
        return {brOut, false};
    }
}

/**
 * We will target LLVM IR.
 */

unsigned int Parse::Sequence::codeGen(CodeGen::IRProg& prog){
    unsigned int accm = 0;
    for(auto iter = this->iterator(); !iter.done(); iter.next()){
        accm += iter.get()->codeGen(prog);
    }
    // unconditional branch
    auto res = getLabel(this->getSequential());
    IInstr uncondBr(SSA::nullValue(), res.first, Label::nullLabel());
    if(!res.second) {
        prog.addInstr(res.first, uncondBr); 
    }
    return 0;
}

/**
 * 1. Get the past SSA name in output. Result of my predicate.
 * 2. Generate this code.
 *
 * Cmp1: %cmp1 = icmp <expr1>
 * br Cmp1 If1 Cmp2
 * Cmp2: %cmp2 = icmp <expr2>
 * br Cmp2 If2 Cmp3
 * ....
 * Else is an unconditional branch if exists.
 *
 * looks like this.
 *
 * Label_1:
 * if(cmp1){
 *    T1a;
 *    T1b;
 *    T1c;
 *    <ControlNode>- transfer control to AST* 
 * }
 *
 * Label_2:
 * whatever's next in the sequence.
 */
unsigned int Parse::Branch::codeGen(IRProg& prog){
    int ctr = 0;
    Utils::SmallVector<
            std::pair<unsigned int, Label>, 4> branches;
    for(auto iter = this->iterator(); !iter.done(); iter.next()){
        // call codegen on the expr.
        unsigned int before = branches.size();
        Branch* br = static_cast<Branch*>(iter.get());
        static_cast<Branch*>(iter.get())->getPred()->codeGen(prog);
        auto ret = getLabel(br->getPred());
        Label next;
        if(ret.second){
            next = ret.first;
        } else {
            Global::specifyError("This branch was previously visited.\n", __FILE__, __LINE__);
            throw Global::DeveloperError;
        }
        prog.associate(next, before);
        prog.allocate(1); // for the branch
        branches.push_back({before,next});
    }

    // the iterator has the very nice property that it stops BEFORE
    // the else condition.
    
    auto iter = this->iterator();
    for(unsigned i = 0; i<branches.size(); ++i){
        unsigned offset = static_cast<Branch*>(iter.get())->codeGen(prog);
        IInstr* toSet = prog.getInstr(i);
        unsigned ptr = prog.size()-offset;
        Label nextLbl = nextLabel();
        prog.associate(nextLbl, ptr);

        toSet->setBranch(*(prog.getInstr(branches[i].first)->getDest()), nextLbl, branches[i+1].second);
        iter.next();
    } 

    return 0;
}

/** Compute the Expr, Setup a loop 
 *
 * 1. Do the branch at the top. If yes, continue to next line. If no, leave loop.
 * 2. At the end of the loop, unconditionally branch to the top comp.
 *
 * Structure:
 * 
 * L1: %s1..... my comparison
 * L2: loop ast....
 *     br L1
 */
unsigned int Parse::Loop::codeGen(IRProg& prog){
    return 0;    
}

unsigned int Parse::Op::codeGen(IRProg& prog){
     
    return 0;
}

CodeGen::SSA genSSA(Parse::Literal* handle){
    if(handle->isInt()){
        return SSA::genUInt(handle->getInt());
    } else {
        // is float
        return SSA::genFlt(handle->getFlt());
    }
}

/** Simple substitution */
unsigned int Parse::Literal::codeGen(IRProg& prog){
    return 0;
}

CodeGen::SSA genSSA(Parse::Variable* handle){
    auto ssa = CodeGen::nextSSA();
    CodeGen::insertVarSSA(handle->getHash(), ssa);
    return ssa;
}

/** Simple substitution...? */
unsigned int Parse::Variable::codeGen(IRProg& prog){
    return 0;
}

SSA exprRecur(Parse::Expr* expr){    
    using Parse::ExprId;
    // cant handle recursively here. mostly..
    switch(expr->exprID()){
        case ExprId::_OP:
            
        case ExprId::_LIT:
            break;
        case ExprId::_VAR:
            break;
    }
    return SSA::nullValue();
}

unsigned int Parse::Expr::codeGen(IRProg& prog){
    using Parse::ExprId;
    using Parse::IntrOps;

    // cant handle recursively here. mostly..
    switch(exprID()){
        case ExprId::_OP:
        {
            Op* op = static_cast<Op*>(this);
            // dont use the iterator bc we need fine grained control.
            switch(op->arity()){
                case 1:
                {
                    /* dest needs to be analyzed */
                    SSA res = exprRecur(op->getUnaryArg());
                    IInstr instr(op->getIntrinsicOp(), VarType::VOID, 
                                    res, nextSSA());
                    prog.addInstr(instr);
                    break;
                }
                case 2:
                {
                    /* Assignment */
                    SSA dest = nextSSA();
                    if(op->getIntrinsicOp() == IntrOps::ASSN){
                        
                    } else {
                    
                    }
                    SSA res1 = exprRecur(op->getBinaryArg1());
                    SSA res2 = exprRecur(op->getBinaryArg2());
                    IInstr instr(op->getIntrinsicOp(), VarType::VOID, 
                                    res1, res2, dest);
                    prog.addInstr(instr);
                    break;
                }
                default:
                    Global::specifyError("Functions (n-ary ops) not supported yet.\n",
                                    __FILE__, __LINE__);
                    throw Global::NotSupportedError;
            }
        } 
        case ExprId::_LIT:
        {
            // base case, an assignment from literal.
            IInstr instr(CodeGen::nextSSA(), 
                            genSSA(static_cast<Literal*>(this)));
            prog.addInstr(instr);
            break;
        }
        case ExprId::_VAR:
        {
            // base case, an assignment from variable.
            IInstr instr(CodeGen::nextSSA(), 
                            genSSA(static_cast<Variable*>(this)));
            prog.addInstr(instr);
            break;
        }
    }
    return 0;
}

/** An alloca <type> */
unsigned int Parse::Decl::codeGen(IRProg& prog){
    IInstr(this->getType(), genSSA(this->getVar()));
    return 1;
}

unsigned int CodeGen::genIR(IRProg& prog){
    Parse::globScope()->getSeq()->codeGen(prog);
    return 0;
}

/*
unsigned int Parse::ControlNode::codeGen(IRProg& prog){
    return 0;
}
*/
