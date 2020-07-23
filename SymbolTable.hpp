
#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "AST.hpp"
#include "Lex.h"
#include <cstddef>

using Parse::Control;
using Parse::Variable;

class ScopedVar {
	friend class Utils::SetTraits<ScopedVar>;
	private:
		Variable* var;
		Control* scope;
	public:
		static char* base;
		ScopedVar(){
			var = &(Parse::_emptyVar);
			scope = nullptr;
		}
		ScopedVar(Variable* _var, Control* _sc){
			var = _var;
			scope = _sc;
		}	
		static inline void setBase(char* prog){
			base = prog;
		}
		static inline char* getBase(){return base;}
		Variable* getVar(){return var;}
		Control* getScope(){return scope;}
};
char* ScopedVar::base = nullptr;

template<>
struct Utils::SetTraits<ScopedVar>{
	static ScopedVar emptyVal(){
		return ScopedVar();
	}
	static ScopedVar tombstoneVal(){
		return ScopedVar(&(Parse::_tombstoneVar), nullptr);
	}
	static size_t hash(ScopedVar v){
		return v.var->namePtr() - ScopedVar::getBase();
	}
	static bool equal(ScopedVar v1, ScopedVar v2){
		return v1.var->namePtr() == v2.var->namePtr();
	}
};

namespace Parse {
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
				table.insert(ScopedVar(v, cntrl));
			}	
			Control* query(Variable* v){
				ScopedVar* sv = table.find(ScopedVar(v, nullptr));
				return sv == nullptr ? nullptr : sv->getScope();	
			}
	};
}

#endif
