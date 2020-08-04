
#include "AST.hpp"
#include "Vector.hpp"
#include "CodeGen.h"

/**
 * We will target LLVM IR.
 */

/** None */
void Parse::Sequence::codeGen(Utils::Vector<CodeGen::IInstr>& output){
	for(auto iter = this->iterator(); !iter.done(); iter.next()){
		iter.get()->codeGen(output);
	}
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
void Parse::Branch::codeGen(Utils::Vector<CodeGen::IInstr>& output){
	
}

/** Compute the Expr, Setup a loop 
 *
 * 1. Get the past SSA 
 */
void Parse::Loop::codeGen(Utils::Vector<CodeGen::IInstr>& output){
	
}

void Parse::Op::codeGen(Utils::Vector<CodeGen::IInstr>& output){
	
}

void Parse::Literal::codeGen(Utils::Vector<CodeGen::IInstr>& output){
	
}

void Parse::Variable::codeGen(Utils::Vector<CodeGen::IInstr>& output){
	
}

void Parse::Decl::codeGen(Utils::Vector<CodeGen::IInstr>& output){
	
}

void genIR(Utils::Vector<CodeGen::IInstr>& buf){
	
}
