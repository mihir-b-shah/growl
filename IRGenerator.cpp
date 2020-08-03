
#include "AST.hpp"
#include "Vector.hpp"
#include "CodeGen.h"

/**
 * We will target LLVM IR.
 *
 *  
 *
 *
 *
 *
 *
 */

/** None */
void Parse::Sequence::codeGen(Utils::Vector<CodeGen::IInstr>& output){
	for(auto iter = this->iterator(); !iter.done(); iter.next()){
		iter.get()->codeGen(output);
	}
}

/**
 * 1. Get the past SSA name in output. Result of my predicate.
 *
 */
void Parse::Branch::codeGen(Utils::Vector<CodeGen::IInstr>& output){

}

/** Compute the Expr, Setup a loop */
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
