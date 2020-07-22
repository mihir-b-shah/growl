
#ifndef GROUP_FINDER_H
#define GROUP_FINDER_H

#include "Set.hpp"
#include "Lex.h"
#include "Error.h"
#include "Vector.hpp"
#include <cstddef>
#include <iostream>

class Match {
	private:
		int _open;
		int _close;
	public:
		Match(){
			_open = -1;
			_close = -1;
		}

		Match(int o, int c){
			_open = o;
			_close = c;
		}

		int open(){return _open;}
		int close(){return _close;}
};

template<>
struct Utils::SetTraits<Match> {
	static Match emptyVal(){return Match(-1,-1);}
	static Match tombstoneVal(){return Match(-2,-2);}
	static size_t hash(Match m){return m.open();}
	static bool equal(Match m1, Match m2){
		return m1.open() == m2.open();
	}
};

namespace Parse {
	
	class GroupFinder {
		private:
			Utils::SmallSet<Match, 200, Utils::SetTraits<Match>> map;
			static inline void throwError(){
				Global::specifyError("Brackets not balanced.\n");
				throw Global::InvalidExpression;
			}
			struct Tkn {
				char token;
				int pos;
			};
		public:
			GroupFinder(Lex::Token* begin, Lex::Token* end){
				Utils::SmallVector<Tkn, 200> stack;
				Lex::Token* ptr = begin;
				int idx = 0;
				while(ptr != end){
					using Lex::SubType;
					switch(ptr->subType){
						case SubType::OBRACK:
							stack.push_back({'{',idx});
							break;
						case SubType::CBRACK:
							if(__builtin_expect(stack.eback().token != '{', false)){
								throwError();
							}
							map.insert(Match(stack.eback().pos, idx));
							stack.pop_back();
							break;
						case SubType::OPAREN:
							stack.push_back({'(',idx});
							break;
						case SubType::CPAREN:
							if(__builtin_expect(stack.eback().token != '(', false)){
								throwError();
							}
							map.insert(Match(stack.eback().pos, idx));
							stack.pop_back();
							break;
						default:
							break;		
					}
					++ptr;
					++idx;
				}	
			}
			~GroupFinder(){}
			int find(int offset, int match){
				Match* mth = map.find({offset+match,0});
				return mth == nullptr ? -1 : mth->close()-offset;
			}	
	};
}

#endif
