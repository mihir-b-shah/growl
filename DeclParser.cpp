
#include "AST.hpp"
#include "Lex.h"
#include "Error.h"
#include "Parse.h"

// WILL CHANGE ONCE WE HAVE OBJECTS
Parse::Variable* Parse::parseDecl(Lex::Token* begin){
	if(__builtin_expect(begin->type != Lex::Type::DATATYPE,false)){
		Global::specifyError("Type not provided.\n");
		throw Global::InvalidDeclaration;
	}		
	Lex::Token* type = begin;
	++begin;
	if(__builtin_expect(begin->type != Lex::Type::ID, false)){
		Global::specifyError("Identifier not present.\n");
		throw Global::InvalidDeclaration;
	}
	int ptrLvl;
	switch(begin->value.iof){
		case Lex::IOF::INT_VAL:
		case Lex::IOF::FLOAT_VAL:
			ptrLvl = 0;
			break;
		case Lex::IOF::PTRLVL:
			ptrLvl = type->value.holder.ptrLvl;
			break;
		default:
			Global::specifyError("IOF has undefined value.\n");
			throw Global::DeveloperError;
	}
	
	auto var = new Parse::Variable(begin->pos, begin->size,	
					type->subType, ptrLvl);
	++begin;
	if(__builtin_expect(begin->type != Lex::Type::GROUP 
			|| begin->subType != Lex::SubType::SEMICOLON, false)){
		Global::specifyError("Semicolon not found\n");
		throw Global::InvalidDeclaration;
	}
	return var;
}
