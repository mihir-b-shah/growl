
#include "AST.hpp"
#include "Lex.h"
#include "Error.h"
#include "Parse.h"
#include "SymbolTable.hpp"
#include <iostream>

// WILL CHANGE ONCE WE HAVE OBJECTS
// guaranteed that *begin is a type.
Lex::Token* Parse::parseDecl(Lex::Token* begin, Parse::Variable* var, Parse::Control* cntrl){
	Lex::Token* type = begin;
	++begin;
	if(__builtin_expect(begin->type != Lex::Type::ID, false)){
		return nullptr;
	}
	int ptrLvl = type->value.holder.ptrLvl;
	var->set(begin->pos, begin->size, type->subType, ptrLvl);
	++begin;

	/**
	 * 2 ways to declare stuff:
	 *
	 * type id;
	 * type id(args); // right now not supported bc function calls not implemented.
	 */

	if(__builtin_expect(begin->type != Lex::Type::GROUP 
			|| begin->subType != Lex::SubType::SEMICOLON, false)){
		Global::specifyError("Semicolon not found\n");
		throw Global::InvalidDeclaration;
	}

	var->debugPrint(std::cout);

	/** WILL NEED TO CHANGE, THIS IS FOR DEBUGGING PURPOSE. */
	Parse::st()->insert(var, Parse::globScope());
	return begin;
}
