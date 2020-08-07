
#include "AST.hpp"
#include "Vector.hpp"
#include "CodeGen.hpp"
#include <utility>

/**
 * We will target LLVM IR.
 */

/** None */
unsigned int Parse::Sequence::codeGen(CodeGen::IRProg prog){
    unsigned int accm = 0;
    for(auto iter = this->iterator(); !iter.done(); iter.next()){
        accm += iter.get()->codeGen(prog);
    }
    // Check if label to return to has already been alloced.
    // FIX THIS.
    CodeGen::Label brOut;
    if(CodeGen::getFromAST(this->getHash()) == CodeGen::Label::nullLabel()){
        // doesnt exist right now.
        CodeGen::insertASTLbl(this->getHash(), brOut = CodeGen::nextLabel());
    } else {
        brOut = CodeGen::getFromAST(this->getHash());
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
 *
 *
 *
 *
 *
 *
 */
unsigned int Parse::Branch::codeGen(CodeGen::IRProg prog){
    int ctr = 0;
    Utils::SmallVector<
            std::pair<unsigned int, CodeGen::Label>, 4> branches;
    for(auto iter = this->iterator(); !iter.done(); iter.next()){
        // call codegen on the expr.
        unsigned int before = branches.size();
        static_cast<Branch*>(iter.get())->getPred()->codeGen(prog);
        CodeGen::Label next = CodeGen::nextLabel();
        prog.associate(next, before);
        prog.allocate(1); // for the branch
        branches.push_back({before,next});
    }

    // the iterator has the very nice property that it stops BEFORE
    // the else condition.
    
    auto iter = this->iterator();
    for(unsigned i = 0; i<branches.size(); ++i){
        unsigned offset = static_cast<Branch*>(iter.get())->codeGen(prog);
        CodeGen::IInstr* toSet = prog.getInstr(i);
        unsigned ptr = prog.size()-offset;
        CodeGen::Label nextLbl = CodeGen::nextLabel();
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
unsigned int Parse::Loop::codeGen(CodeGen::IRProg prog){
    return 0;    
}

/** Setup an operator 
 * 
 * Call codegen on each of the supporting operators.
 * then get the % node for each of them. Then call my operator.
 */
unsigned int Parse::Op::codeGen(CodeGen::IRProg prog){
    return 0;
}

/** Simple substitution */
unsigned int Parse::Literal::codeGen(CodeGen::IRProg prog){
    return 0;
}

/** Simple substitution...? */
unsigned int Parse::Variable::codeGen(CodeGen::IRProg prog){
    return 0;
}

/** An alloca <type> */
unsigned int Parse::Decl::codeGen(CodeGen::IRProg prog){
    return 0;
}

unsigned int genIR(Utils::Vector<CodeGen::IInstr>& buf){
    return 0;
}
