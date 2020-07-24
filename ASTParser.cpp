
#include "AST.hpp"
#include "Parse.h"
#include "Lex.h"
#include "Error.h"
#include "Syntax.h"
#include <iostream>

using namespace Parse;

static inline void unsupported(){
	Global::specifyError("Not supported yet.");
	throw Global::DeveloperError;
}

static AST* parse(int offset, Lex::Token* begin, Lex::Token* end, Control* cntrl){
	using Lex::SubType;
	using Lex::Type;

	if(begin >= end) return nullptr;

	switch(begin->type){
		case Type::DATATYPE:
		{
			Variable* var = new Variable();
			// guaranteed that semicolon is found.
			Lex::Token* ret = 1+parseDecl(begin, var, cntrl);
			// POTENTIAL FOR STACK OVERFLOW IF THIS
			// RECURSION GOES TOO DEEP
			if(__builtin_expect(ret == nullptr, false)){
				// Specifically, this is bc constructors dont exist now.
				unsupported();
			}
			return parse(offset + (ret-begin), ret, end, cntrl);
		}
		case Type::OPERATOR:
			if(Syntax::opType(begin->subType) == Syntax::OpType::BINARY){
				Global::specifyError(
								"Presence of binary operator invalidates expr.\n");
				throw Global::InvalidExpression;
			}
		case Type::ID:
		case Type::LITERAL:
		{
			Lex::Token* runner = begin;
			while(runner != end && runner->subType != SubType::SEMICOLON){
				++runner;
			}	
			if(runner == end){
				Global::specifyError("Semicolon not found.\n");
				throw Global::InvalidExpression;
			}
			Expr* expr = parseExpr(begin, runner);
			expr->print(80, std::cout);
			AST* next = parse(offset + (runner-begin) + 1, runner + 1, end, cntrl);	
			return joinAST(expr, next);
		}
		case Type::CONTROL:
		{
			if(__builtin_expect(begin->subType != SubType::WHILE, false)){
				unsupported();
			}
			Loop* loop = new Loop();
			Lex::Token* next = parseLoop(offset, begin, loop); 
			AST* rest = parse(offset + (next - begin), next, end, cntrl);
			return joinAST(loop, rest); 
		}
		case Type::GROUP:
			if(__builtin_expect(begin->subType != SubType::CBRACK, false)){
				unsupported();
			}
			break;
	}
	return nullptr;
}

/** Stack overflow warning */
AST* Parse::parseAST(int offset, Lex::Token* begin, Lex::Token* end, Control* cntrl){
	return parse(offset, begin, end, cntrl);
}
