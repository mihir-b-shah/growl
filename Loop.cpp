
#include "Parse.h"
#include "AST.hpp"
#include "Lex.h"
#include "Error.h"
#include <iostream>

using namespace Parse;

/**
 * Syntax:
 * while (bool predicate) {
 * 	   some AST.
 * }
 *
 *
 */
Lex::Token* Parse::parseLoop(int offset, Lex::Token* begin, Loop* lp){
	
	int idx = 0;
	if(__builtin_expect((begin+idx)->subType != SubType::WHILE,false)){
		Global::specifyError("\"While\" token not found at loop start.\n");
		throw Global::InvalidLoop;
	}
	++idx;
	if(__builtin_expect((begin+idx)->subType != SubType::OPAREN,false)){
		Global::specifyError("Opening paren not found after while.\n");
		throw Global::InvalidLoop;
	}
	int matchParen = gf()->find(offset,idx);
	Expr* predicate = parseExpr(begin+idx+1, begin+matchParen);
	lp->setPred(predicate);
	lp->setBracket(offset+const_cast<char*>(begin->pos));

	idx = matchParen+1;
	if(__builtin_expect((begin+idx)->subType != SubType::OBRACK, false)){
		Global::specifyError("Opening bracket not found after while.\n");
		throw Global::InvalidLoop;
	}

	int matchBrack = gf()->find(offset, idx);
	parseAST(offset + matchParen + 2, begin + matchParen + 2, 
					begin + matchBrack, lp);
    return begin+matchBrack+1;
}
