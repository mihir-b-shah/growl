
#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "AST.hpp"
#include "Lex.h"
#include <cstddef>
#include "Set.hpp"

using Parse::Variable;
using Parse::Control;

class ScopedVar {
	friend class Utils::SetTraits<ScopedVar>;
	private:
		Variable* var;
		Control* scope;
	public:
		ScopedVar(){
			var = Parse::emptyVar();
			scope = nullptr;
		}
		ScopedVar(Variable* _var, Control* _sc){
			var = _var;
			scope = _sc;
		}	
		Variable* getVar(){return var;}
		Control* getScope(){return scope;}
};

template<>
struct Utils::SetTraits<ScopedVar>{
	static ScopedVar emptyVal(){
		return ScopedVar();
	}
	static ScopedVar tombstoneVal(){
		return ScopedVar(Parse::tombsVar(), nullptr);
	}
	static size_t hash(ScopedVar v){
		return v.var->namePtr() - Lex::program();
	}
	static bool equal(ScopedVar v1, ScopedVar v2){
		return v1.var->namePtr() == v2.var->namePtr();
	}
};

namespace Parse {
	/**
	 * Solving the duplicate variable problem efficiently
	 *
	 * The char* pointer WORKS except: when i want to query, how do i find this?
	 * Use an auxilliary map (array). Map: string -> char*.
	 * And the final lookup map: char* -> char*.
	 *
	 * This can be built with scanning algorithm
	 *
	 * Stack<char*>. When I enter a scope. We push the declaration I encountered on.
	 * 				 When I leave one, pop the stack until all declarations from that stack gone.
	 *
	 * As I go through the document, updating the map. 
	 * If I find any identifier. Hash it (as a string) and see if it already exists.
	 * If it does, then its already been declared. Put in lookup the id's address char* and map it to the existing entry.
	 * If it doesn't, map it to itself in lookup. Also add it to the auxilliary map (string->char*)
	 *
	 * When I exit a scope, as I pop from the stack, remove from the auxilliary array.
	 *
	 * Net result is the final lookup map. Resolves any problems.
	 *
	 * Optimizations:
	 * when using the map: string -> char*. Instead of storing the strings over and over
	 * define a StringView class that takes a string at some position. 
	 *
	 * class StringView {
	 *      char* front;
	 *      int len;
	 *      int hash;
	 * }
	 *
	 */
	class SymbolTable {
		private:
			Utils::SmallSet<ScopedVar, 200, Utils::SetTraits<ScopedVar>> table;	
		public:
			// doing multiple passes over the file isn't ideal but its probably fine here.
			SymbolTable(){
			}
			~SymbolTable(){
			}
			void insert(Variable* v, Control* cntrl){
				std::cout << (v->namePtr()) << '\n';
				std::cout << (void*) (v->namePtr()) << '\n';
				table.insert(ScopedVar(v, cntrl));
			}	
			Variable* query(unsigned char len, const char* ptr){
				std::cout << (ptr) << '\n';
				std::cout << ((void*) ptr) << '\n';
				Variable v = Variable(ptr, len, 
								Lex::SubType::VOID, 0);
				// last two values in v are nonsense.
				ScopedVar* sv = table.find(ScopedVar(&v, nullptr));
				return sv == nullptr ? nullptr : sv->getVar();
			}
			Control* query(Variable* v){
				ScopedVar* sv = table.find(ScopedVar(v, nullptr));
				return sv == nullptr ? nullptr : sv->getScope();	
			}
	};
}

#endif
