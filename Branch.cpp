
#include "Parse.h"
#include "AST.hpp"
#include "Lex.h"
#include "Error.h"
#include <iostream>

using namespace Parse;

/**
 * Syntax:
 * if (bool predicate){
 *     some AST.
 * }
 *
 */
Lex::Token* Parse::parseBranch(int offset, Lex::Token* begin, Branch* br){
	
	int idx = 0;
	if(__builtin_expect((begin+idx)->subType != SubType::IF,false)){
		Global::specifyError("\"If\" token not found at branch start.\n");
		throw Global::InvalidBranch;
	}
	++idx;
	if(__builtin_expect((begin+idx)->subType != SubType::OPAREN,false)){
		Global::specifyError("Opening paren not found after if.\n");
		throw Global::InvalidBranch;
	}
	int matchParen = gf()->find(offset,idx);
	Expr* predicate = parseExpr(begin+idx+1, begin+matchParen);
	br->setPred(predicate);
	br->setBracket(offset+const_cast<char*>(begin->pos));

	idx = matchParen+1;
	if(__builtin_expect((begin+idx)->subType != SubType::OBRACK, false)){
		Global::specifyError("Opening bracket not found after if.\n");
		throw Global::InvalidBranch;
	}

	int matchBrack = gf()->find(offset, idx);
	parseAST(offset + matchParen + 2, begin + matchParen + 2, 
					begin + matchBrack, br);
    return begin+matchBrack+1;
}
