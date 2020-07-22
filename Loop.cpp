
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
Loop* Parse::parseLoop(int offset, Lex::Token* begin, Lex::Token* end){
	
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
	std::cout << matchParen << '\n'; 
	Expr* predicate = parseExpr(begin+idx+1, begin+matchParen);
	predicate->print(80, std::cout);

	idx = matchParen+1;
	if(__builtin_expect((begin+idx)->subType != SubType::OBRACK, false)){
		Global::specifyError("Opening bracket not found after while.\n");
		throw Global::InvalidLoop;
	}

	int matchBrack = gf()->find(offset, idx);
	std::cout << matchBrack << '\n';

	// NOT THE CASE ALWAYS JUST FOR DEMONSTRATION.
	std::cout << idx+1 << ' ' << matchBrack << '\n';
	Expr* body = parseExpr(begin+idx+1, begin+matchBrack-1);
	body->print(80, std::cout);	

    return nullptr;
}
