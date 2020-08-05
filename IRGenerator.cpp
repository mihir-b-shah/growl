
#include "AST.hpp"
#include "Vector.hpp"
#include "CodeGen.hpp"

/**
 * We will target LLVM IR.
 */

/** None */
unsigned int Parse::Sequence::codeGen(Utils::Vector<CodeGen::IInstr>& output){
	unsigned int accm = 0;
	for(auto iter = this->iterator(); !iter.done(); iter.next()){
		accm += iter.get()->codeGen(output);
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
 */
unsigned int Parse::Branch::codeGen(Utils::Vector<CodeGen::IInstr>& output){
	for(auto iter = this->iterator(); !iter.done(); iter.next()){

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
unsigned int Parse::Loop::codeGen(Utils::Vector<CodeGen::IInstr>& output){
	return 0;	
}

/** Setup an operator 
 * 
 * Call codegen on each of the supporting operators.
 * then get the % node for each of them. Then call my operator.
 */
unsigned int Parse::Op::codeGen(Utils::Vector<CodeGen::IInstr>& output){
	return 0;
}

/** Simple substitution */
unsigned int Parse::Literal::codeGen(Utils::Vector<CodeGen::IInstr>& output){
	return 0;
}

/** Simple substitution...? */
unsigned int Parse::Variable::codeGen(Utils::Vector<CodeGen::IInstr>& output){
	return 0;
}

/** An alloca <type> */
unsigned int Parse::Decl::codeGen(Utils::Vector<CodeGen::IInstr>& output){
	return 0;
}

unsigned int genIR(Utils::Vector<CodeGen::IInstr>& buf){
	return 0;
}
