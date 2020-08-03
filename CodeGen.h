
#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "Vector.hpp"

namespace CodeGen {
	
	enum class _IInstr {
	};

	enum class Register {
		R0, R1, R2, R3, R4, R5, R6, R7
	};

	enum MemLoc {
		REG, MEM
	};

	class Address {
		MemLoc ml;
		union {
			Register reg;
		} addr;	
	};

	class IInstr {
		_IInstr ins;
			
	};	

	void genASM(Utils::Vector<IInstr>& buf);
}

#endif


