
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

static inline Label uncheckedGetLabel(Parse::AST* ast){
    Label brOut = nextLabel();
    insertASTLbl(ast->getHash(), brOut);
    return brOut;
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
    unsigned accm = 0;
    // where I put the branches' indexes that need to be filled.
    // 10 because its a pretty high upper bound.
    Utils::SmallVector<unsigned,10> branches; 
    Parse::AST* next = this->getSeq()->getSequential();

    // Add the unconditional branch at the start.
    auto topRes = getLabel(this);
    if(topRes.second){
        IInstr uncondBr(SSA::nullValue(), topRes.first, Label::nullLabel());
        prog.addInstr(uncondBr);
        ++accm;
    }
    
    // Iterator stops one early, i.e. it ignores the else branch.
    for(auto iter = this->iterator(); !iter.done(); iter.next()){
        /* we get each of the branches and do codegen on their predicates,
         * and get their branches setup. These branches' pos are in lbls. */
        Branch* br = static_cast<Branch*>(iter.get());
        Label exprLbl = getLabel(br).first;
        prog.associate(exprLbl, prog.size());
        unsigned moveBack = br->getPred()->codeGen(prog);
        accm += moveBack;
        IInstr brInstr(*(prog.getLastInstr()->getDest()), 
                        Label::nullLabel(), Label::nullLabel());
        // pair: the index of the comparison, the index of the subseq. branch
        branches.push_back(prog.size());
        prog.addInstr(brInstr);
    }

    // now time to get the labels from codegen on ASTs.
    // this code tries to fill: br %c1 If1 %c2.
 
    Utils::SmallVector<unsigned,11> seqBranches; 
    // reuse this buffer.
    auto iter = this->iterator(); 
    int ctr = 0;

    for(; !iter.done(); iter.next()){
        Branch* br = static_cast<Branch*>(iter.get());

        // get the label for the sequence.
        Label astLbl = getLabel(br->getSeq()).first;
        prog.associate(astLbl, prog.size());

        // set the if branch.
        prog.getInstr(branches[ctr])->setIfBr(astLbl);
        
        // generate the code for the sequence. 
        unsigned seqCodeLen = br->getSeq()->codeGen(prog);

        // Set the else condition on each instruction-level branch.
        Label nextPredLbl = getLabel(br->getNext()->getSeq()).first;
        prog.associate(nextPredLbl, prog.size());
        prog.getInstr(branches[ctr])->setElseBr(nextPredLbl);
        
        // the sequential element. Get the branch to it.
        IInstr seqBr(SSA::nullValue(), Label::nullLabel(), Label::nullLabel());
        branches.push_back(prog.size());
        prog.addInstr(seqBr);

        // Handle the else branch.
        if(br->getNext()->getPred() == nullptr){
            Label elseLbl = getLabel(br->getNext()->getSeq()).first;
            prog.associate(elseLbl, prog.size());
            prog.getInstr(branches[ctr+1])->setElseBr(elseLbl);
            br->getNext()->getSeq()->codeGen(prog); 

            // the sequential element for else. Get the branch to it.
            IInstr eseqBr(SSA::nullValue(), Label::nullLabel(), Label::nullLabel());
            branches.push_back(prog.size());
            prog.addInstr(eseqBr);
            
            break; 
        } 

        ++ctr;
    }

    Label nextLbl = getLabel(next).first;
    for(auto iter = branches.begin(); iter != branches.end(); ++iter){
        prog.getInstr(*iter)->setIfBr(nextLbl);
    }
    prog.associate(nextLbl, prog.size());

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
    // Generate expr code and label its start
    unsigned accm = 0;
    auto exprRes = getLabel(this);
    Label exprLbl = exprRes.first;

    if(exprRes.second){
        /* we just inserted this label, we need to add an 
         * unconditional branch to it to define it */
        IInstr uncondBr(SSA::nullValue(), exprLbl, Label::nullLabel());
        prog.addInstr(uncondBr);
        ++accm;
    }

    unsigned exprPos = this->getPred()->codeGen(prog);
    accm += exprPos;
    exprPos = prog.size()-exprPos;
    prog.associate(exprLbl, exprPos);
  
    // Allocate the initial branch, we'll fill it in later. 
    prog.allocate(1);
    accm += 1;
    
    // Generate loop body's code and label its start. 
    unsigned astPos = this->getSeq()->codeGen(prog);
    accm += astPos;
    astPos = prog.size()-astPos; 

    // guaranteed not to have been reached yet.
    Label astLbl = uncheckedGetLabel(this->getSeq());
    prog.associate(astLbl, astPos); 

    // Add an unconditional branch back to the expr.
    IInstr uncondBr(SSA::nullValue(), exprLbl, Label::nullLabel());
    prog.addInstr(uncondBr);
    accm += 1;

    /* Fill in that initial branch.
     * Notice, no one has actually filled the sequentially next thing,
     * but it should still work. */
    Parse::AST* next = this->getSeq()->getSequential();
   
    // It is guaranteed that 
    auto nextRes = getLabel(next);
    if(!nextRes.second){
        Global::specifyError("Next AST in sequence already modified.\n",
                        __FILE__, __LINE__);
        throw Global::DeveloperError;
    }
    Label seqNext = nextRes.first;
    prog.associate(seqNext, prog.size());
    prog.getInstr(astPos-1)->setBranch(*(prog.getInstr(astPos-2)->getDest()), 
                    astLbl, seqNext); 
    
    return accm;    
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

