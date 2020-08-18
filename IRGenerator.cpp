
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
    return accm;
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
 * L1: %s1 = <res of expr>
 *     br %s1 L2 L3  
 * L2: ......
 *     ......
 *     br L1
 * L3: NextAST
 */
unsigned int Parse::Loop::codeGen(IRProg& prog){
    Label exprLbl = nextLabel();
    unsigned exprPos = prog.size()-this->getPred()->codeGen(prog);
    prog.associate(exprLbl, exprPos);
    
    Label astLbl = nextLabel();
    prog.allocate(1);
    unsigned astPos = prog.size()-this->getSeq()->codeGen(prog);
    prog.associate(astLbl, astPos); 

    /* THIS IS WRONG WE WANT TO POINT TO THE EXPR */
    auto res = getLabel(this->getSeq()->getSequential());
    if(__builtin_expect(!res.second,false)){
        Global::specifyError("Loop body has already been labeled.\n", __FILE__, __LINE__);
        throw Global::DeveloperError;
    }

    prog.getInstr(astPos-1)->setBranch(*(prog.getLastInstr()->getDest()), astLbl, res.first); 
    IInstr uncondBr(SSA::nullValue(), exprLbl, Label::nullLabel());
    prog.addInstr(uncondBr);

    /* NEED TO HANDLE THE OUTBOUND CASE WITH L3 */
    return 0;    
}

CodeGen::SSA genSSALit(Parse::Literal* handle){
    if(handle->isInt()){
        return SSA::genUInt(handle->getInt());
    } else {
        // is float
        return SSA::genFlt(handle->getFlt());
    }
}

CodeGen::SSA genSSA(Parse::Expr* handle){
    auto ssa = CodeGen::nextSSA();
    CodeGen::insertVarSSA(handle->getHash(), ssa);
    return ssa;
}

unsigned int castCodeGen(Parse::Cast* cast, IRProg& prog){
    SSA ssaIn = getFromVar(cast->getExpr()->getHash());
    SSA ssaOut = genSSA(cast);
    IInstr instr(cast->getExpr()->getType(), cast->getCastType(),
                    ssaIn, ssaOut);
    prog.addInstr(instr);
    return 1 + instr.isTwoInstrCast(); // bool implicit cast.
}

static inline void funcNotSupported(){
    Global::specifyError("Function (n-ary op) not supported", 
                    __FILE__, __LINE__);
    throw Global::NotSupportedError;
}

static SSA polymorphGetSSA(Parse::Expr* expr){
    if(expr->exprID() == Parse::ExprId::_LIT){
        return genSSALit(static_cast<Parse::Literal*>(expr));
    } else {
        return getFromVar(expr->getHash()); 
    }
}

unsigned int opCodeGen(Parse::Op* op, IRProg& prog){
    if(__builtin_expect(!(op->isIntrinsic()), false)){
        funcNotSupported();
    }
    switch(op->arity()){
        case 1:
        {
            SSA ssa = polymorphGetSSA(op->getUnaryArg());
            IInstr instr(op->getIntrinsicOp(), op->getType(),
                           ssa, genSSA(op));
            prog.addInstr(instr);
            return 1; 
        }
        case 2:
        {
            SSA ssa1 = polymorphGetSSA(op->getBinaryArg1());
            SSA ssa2 = polymorphGetSSA(op->getBinaryArg2());
           
            // i.e, its a literal where assignment ops need to be reversed. 
            if(op->getIntrinsicOp() == IntrOps::ASSN){
                IInstr store(MemAction::STORE, op->getType(), ssa2, 
                                getVarPtr(op->getBinaryArg1()->getHash()));
                IInstr load(MemAction::LOAD, op->getType(), 
                                getVarPtr(op->getBinaryArg1()->getHash()), 
                                genSSA(op->getBinaryArg2()));
                prog.addInstr(store);
                prog.addInstr(load);
                return 2;
            } else {
                IInstr instr(op->getIntrinsicOp(), op->getType(),
                           ssa1, ssa2, genSSA(op));
                prog.addInstr(instr);
                return 1;
            }
        }
        default:
            // already handled by assert at top.
            break;
    }
    return 0;
}

unsigned int polymorphExprCodeGen(Parse::Expr* expr, IRProg& prog){
    using namespace Parse;
    switch(expr->exprID()){
        case ExprId::_OP:
            return opCodeGen(static_cast<Op*>(expr), prog);
        case ExprId::_CAST:
            return castCodeGen(static_cast<Cast*>(expr), prog);
        default:
            return 0;
    }
}

// enum class ExprId:char {_OP, _LIT, _VAR, _CAST};
unsigned int exprRecur(Parse::Expr* expr, IRProg& prog){    
    using namespace Parse;
    // post order traversal.
    unsigned accm = 0;
    for(auto iter = expr->iterator(); !iter.done(); iter.next()){
        accm += exprRecur(static_cast<Expr*>(iter.get()), prog);
    }
    return accm + polymorphExprCodeGen(expr, prog);
}

// ONLY POINT OF ENTRY FOR EXPR CODEGEN.
unsigned int Parse::Expr::codeGen(IRProg& prog){
    using Parse::ExprId;
    using Parse::IntrOps;

    // cant handle recursively here. mostly..
    switch(exprID()){
        case ExprId::_OP:
        case ExprId::_CAST:
        {
            return exprRecur(this, prog);
        }
        case ExprId::_LIT:
        {
            // base case, an assignment from literal.
            IInstr instr(this->getType(), CodeGen::nextSSA(), 
                            genSSA(static_cast<Literal*>(this)));
            prog.addInstr(instr);
            return 1;
        }
        case ExprId::_VAR:
        {
            // base case, an assignment from variable.
            return 0;
        }
    }

    return 0;
}

/** An alloca <type> */
unsigned int Parse::Decl::codeGen(IRProg& prog){
    SSA ssa;
    IInstr instr(this->castType(), 
                    ssa = CodeGen::getVarPtr(this->getVar()->getHash()));
    prog.addInstr(instr);
    // add a load for reference.
    IInstr instr2(MemAction::LOAD, this->getVar()->getType(),
                    ssa, genSSA(this->getVar()));
    prog.addInstr(instr2);
    return 2;
}

unsigned int CodeGen::genIR(IRProg& prog){
    Parse::globScope()->getSeq()->codeGen(prog);
    return 0;
}

unsigned int cntrlNodeCodeGen(Parse::ControlNode* node, IRProg& prog){
    // unconditional branch
    auto res = getLabel(node->getSequentialBase());
    IInstr uncondBr(SSA::nullValue(), res.first, Label::nullLabel());
    if(!res.second) {
        prog.addInstr(res.first, uncondBr); 
        return 1;
    }
    return 0;
}

