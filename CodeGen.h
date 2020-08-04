
#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "Vector.hpp"

namespace CodeGen {
	
	class IInstr {
	};

	class _Branch {
			
	};	

	// my next ssa is "s" plus this number.
	unsigned long long nextSSA();

	void genASM(Utils::Vector<IInstr>& buf);
}

#endif


