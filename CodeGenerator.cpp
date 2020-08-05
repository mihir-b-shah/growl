
#include "CodeGen.hpp"
#include "Vector.hpp"
#include "AST.hpp"

using namespace CodeGen;

void traverse(Parse::AST* ast){
}

/*
 * Example program (loop.grr)
 *
 * int x;
 * x = 0;
 * 
 * while(x < 5) {
 * 	x = x + 1;
 * }
 *
 * Simple ASM (LC3 style):
 *
 * start
 * DECL x
 * ASSN 0 x
 * 
 * _L1:
 *  CMP x 5 %r1
 *  BRzp %r1 _L2
 *  ADD x 1 %r1
 *  ASSN %r1 x
 *  BR _L1
 *
 *_L2:
 *  exit
 */

void genASM(Utils::Vector<IInstr>& instrs){
	traverse(Parse::globScope()->getSeq());
}

