
#ifndef CODE_GEN_HPP
#define CODE_GEN_HPP

#include "Lex.h"
#include "Vector.hpp"
#include "Error.h"
#include <cstdio>
#include <utility>

namespace CodeGen {

	using Lex::SubType;

	enum class LType:char {FLT_LIT, SIGN_LIT, USIGN_LIT, LBL};
	
	/** Label is rep as L1 for instance */
	struct Label {

		// passing 
		union {
			unsigned int lbl;
			long long sint;
			unsigned long long uint;
			long double flt;
		} holder;
		LType which;

		Label(){holder.lbl = 0; which = LType::LBL;}

		Label(unsigned int _lbl){
			holder.lbl = _lbl;
			which = LType::LBL;
		}

		Label operator++(int){
			if(__builtin_expect(which == LType::LBL,true)){
				return Label(holder.lbl++);
			} else {
				Global::specifyError("Label increment called on literal.\n");
				throw Global::DeveloperError;
			}
		}

		static Label genUInt(unsigned long long v){
			Label lbl;
			lbl.which = LType::USIGN_LIT;
			lbl.holder.uint = v;
			return lbl;
		}

		static Label genSInt(long long v){
			Label lbl;
			lbl.which = LType::SIGN_LIT;
			lbl.holder.sint = v;
			return lbl;
		}
		
		static Label genFlt(long double v){
			Label lbl;
			lbl.which = LType::FLT_LIT;
			lbl.holder.flt = v;
			return lbl;
		}

		bool isNull(){return which == LType::LBL && holder.lbl== 0;} 
		unsigned int extractLbl(){return holder.lbl;}
		long long extractSignedInt(){return holder.sint;}
		unsigned long long extractUnsignedInt(){return holder.sint;}
		long double extractFlt(){return holder.flt;}
	};

	/* SSA is rep as %s1 for instance, good type safety too!*/
	struct SSA {
		unsigned long long ssa;

		SSA(){ssa = 0;}

		SSA(unsigned long long _ssa){
			ssa = _ssa;
		}

		SSA operator++(int){
			return SSA(ssa++);
		}

		bool operator==(const SSA& s2){
			return ssa == s2.ssa;
		}

		static SSA nullSSA(){return SSA(0);}
		unsigned int extract(){return ssa;}
	};

	static const unsigned int INSTR_BUF_SIZE = 57;
	static const char* const ERROR_INSTR = "NEVER USED. ERROR.";
	constexpr unsigned int instrBufSize(){return INSTR_BUF_SIZE;}

	struct OpUtils {
		static LType genLType(SubType type){
			switch(type){
				case SubType::CHAR:
				case SubType::LONG:
				case SubType::BOOL:
					return LType::USIGN_LIT;
				case SubType::INT:
					return LType::SIGN_LIT;
				case SubType::FLOAT:
					return LType::FLT_LIT;
				case SubType::VOID:
				default:
					Global::specifyError("Type void or other passed to IR generator.\n");
					throw Global::DeveloperError;
			}
		}
		
		/** Returns 0 if should be signed */
		static unsigned long long genAllOne(SubType type){
			switch(type){
				case SubType::CHAR:
					return 0xff;
				case SubType::LONG:
					return 0xffffffffffffffffULL;
				case SubType::BOOL:
					return 1;
				case SubType::INT:
					return 0;
				case SubType::FLOAT:
				case SubType::VOID:
				default:
					Global::specifyError("Non integer type passed.\n");
					throw Global::DeveloperError;
			}
		}

		static inline const char* formatSpec(LType lt){
			switch(lt){	
				case LType::FLT_LIT: 
					return "Lf";
				case LType::LBL:
					return "u";
				case LType::SIGN_LIT:
					return "lld";
				case LType::USIGN_LIT:
					return "llu";
			}
		}
	};	

	// inheritance hierarchy for all the different inclassions	
	class IInstr {
		virtual void output(char buf[INSTR_BUF_SIZE]) = 0;
	};

	class IBranch : public IInstr {
		SSA pred;
		Label ifbr;
		Label elsebr;

		IBranch(SSA _pred, Label _ifbr, Label _elsebr){
			pred = _pred;
			ifbr = _ifbr;
			elsebr = _elsebr;
		}

		void output(char buf[INSTR_BUF_SIZE]){
			if(pred == SSA::nullSSA()) { 
				std::snprintf(buf, INSTR_BUF_SIZE, "br L%u", ifbr.extractLbl());
			} else {
				std::snprintf(buf, INSTR_BUF_SIZE, "br i1 s%u, label L%u, label L%u",
								pred.extract(), ifbr.extractLbl(), elsebr.extractLbl()); 
			}
		}	
	};

	class IBinOp : public IInstr {
		private:
			enum:char {LIT_LIT_INT, LIT_LIT_FLT, LIT_LBL_INT, LIT_LBL_FLT, 
						LBL_LIT_INT, LBL_LIT_FLT, LBL_LBL_INT, LBL_LBL_FLT};
		protected:
			SubType type;
			Label src1;
			Label src2;
			Label dest;

			IBinOp(SubType _width, Label _src1, Label _src2, Label _dest){
				type = _width;
				src1 = _src1;
				src2 = _src2;
				dest = _dest;
			}

			void assertTypeError(SubType type) {				
				// these are the unsigned types right now.
				LType corresp = OpUtils::genLType(type);
				if(__builtin_expect(dest.which != LType::LBL && (src1.which != corresp 
						|| src1.which != LType::LBL) && (src2.which != corresp 
						|| src2.which != LType::LBL),false)){
					Global::specifyError("Incorrect types passed to IR generator.\n");
					throw Global::DeveloperError;
				}
			}

			/** "instr" should be followed by space.*/
			void printOp(char buf[INSTR_BUF_SIZE], const char* const instr, 
							const char* const dType, bool sign, bool flt) {
				
				const char* spec1 = OpUtils::formatSpec(src1.which);
				const char* spec2 = OpUtils::formatSpec(src2.which);

				// the max number of chars it can be.
				constexpr int FSBUF_LEN = 22;
				
				char fsbuf[FSBUF_LEN] = {'\0'};
				// setup the printf. bad for efficiency but whatever...
				int chk = std::snprintf(fsbuf, FSBUF_LEN, "s%%u = %%s%%s %%%s %%%s",
								spec1, spec2);
				
				if(chk >= FSBUF_LEN) {
					Global::specifyError("Buffer for snprintf too small.\n");
					throw Global::DeveloperError;
				}
				
				// example fsbuf later: "s%u = %s %s %lld %lld"

				switch(((src1.which == LType::LBL) << 2) + ((src2.which == LType::LBL) << 1) + flt){
					case LIT_LIT_INT:
						chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, dType, 
							sign ? src1.extractSignedInt() : src1.extractUnsignedInt(),
							sign ? src2.extractSignedInt() : src2.extractUnsignedInt());
						break;
					case LIT_LIT_FLT:
						chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, dType, 
							src1.extractFlt(), src2.extractFlt());
						break;
					case LIT_LBL_INT:
						chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, dType, 
							sign ? src1.extractSignedInt() : src1.extractUnsignedInt(), src2.extractLbl());
						break;
					case LIT_LBL_FLT:
						chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, dType, 
							sign ? src1.extractFlt() : src2.extractLbl());
						break;
					case LBL_LIT_INT:
						chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, dType, 
							src1.extractLbl(), sign ? src2.extractSignedInt() : src2.extractUnsignedInt());
						break;
					case LBL_LIT_FLT:
						chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, dType, 
							src1.extractLbl(), src2.extractFlt());
						break;
					case LBL_LBL_INT:
					case LBL_LBL_FLT:
						chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, dType, 
							src1.extractLbl(), src2.extractLbl());
						break;
					default:
						Global::specifyError("Bool mask overflowed.\n");
						throw Global::DeveloperError;			
				}

				if(chk >= INSTR_BUF_SIZE){
					Global::specifyError("Buffer for snprintf too small.\n");
					throw Global::DeveloperError;			
				}
			}

			void outputHelp(char buf[INSTR_BUF_SIZE], const char* const unsignInstr, 
							const char* const signInstr, const char* const fltInstr){
				assertTypeError(type);
				switch(type){
					case SubType::CHAR:
						printOp(buf, unsignInstr, "i8", false, false);	
						break;
					case SubType::INT:
						printOp(buf, signInstr, "i32", true, false);
						break;
					case SubType::LONG:
						printOp(buf, unsignInstr, "i64", false, false);
						break;
					case SubType::BOOL:
						printOp(buf, unsignInstr, "i1", false, false);
						break;
					case SubType::FLOAT:
						printOp(buf, fltInstr, "", true, true);
						break;
					default:
						Global::specifyError("Invalid type encountered.\n");
						throw Global::DeveloperError;
				}	
			}	
		public:
			virtual void output(char buf[INSTR_BUF_SIZE]);
	};

	class IAdd : public IBinOp {
		public:
			IAdd(SubType _width, Label _src1, Label _src2, Label _dest)
				  	: IBinOp(_width, _src1, _src2, _dest) {
			}
			
			void output(char buf[INSTR_BUF_SIZE]){
				return outputHelp(buf, "add ", "add ", "fadd ");
			}
	};

	class ISub : public IBinOp {
		public:
			ISub(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			void output(char buf[INSTR_BUF_SIZE]){
				return outputHelp(buf, "sub ", "sub ", "fsub ");	
			}
	};

	class IMul : public IBinOp {
		public:
			IMul(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			void output(char buf[INSTR_BUF_SIZE]){
				return outputHelp(buf, "mul ", "mul ", "fmul ");
			}
	};

	class IDiv : public IBinOp {
		public:
			IDiv(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			void output(char buf[INSTR_BUF_SIZE]){
				return outputHelp(buf, "udiv ", "sdiv ", "fdiv ");
			}
	};

	class IMod : public IBinOp {	
		public:
			IMod(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			void output(char buf[INSTR_BUF_SIZE]){
				return outputHelp(buf, "urem ", "srem ", "frem ");	
			}
	};

	class IShiftLeft : public IBinOp {
		public:
			IShiftLeft(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			void output(char buf[INSTR_BUF_SIZE]){
				return outputHelp(buf, "shl ", "shl ", ERROR_INSTR);	
			}
	};

	class IShiftRight : public IBinOp {
		public:
			IShiftRight(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			void output(char buf[INSTR_BUF_SIZE]){
				return outputHelp(buf, "lshr ", "ashr ", ERROR_INSTR);	
			}
	};

	class IAnd : public IBinOp {
		public:
			IAnd(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			void output(char buf[INSTR_BUF_SIZE]){
				return outputHelp(buf, "and ", "and ", ERROR_INSTR);
			}
	};

	class IOr : public IBinOp {
		public:
			IOr(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			void output(char buf[INSTR_BUF_SIZE]){
				return outputHelp(buf, "or ", "or ", ERROR_INSTR);
			}
	};

	class IXor : public IBinOp {
		public:
			IXor(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			void output(char buf[INSTR_BUF_SIZE]){
				return outputHelp(buf, "xor ", "xor ", ERROR_INSTR);
			}
	};

	class IUnOp : public IInstr {
		protected:
			SubType type;
			Label src;
			Label dest;
	
			IUnOp(SubType _width, Label _src, Label _dest){
				type = _width;
				src = _src;
				dest = _dest;
			}
			
		public:
			virtual void output(char buf[INSTR_BUF_SIZE]);
	};

	// not done yet
	class IGreater : public IBinOp {
		public:
			IGreater(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			void output(char buf[INSTR_BUF_SIZE]){
				return outputHelp(buf, "xor ", "xor ", ERROR_INSTR);
			}
	};

	class ILess : public IBinOp {
		public:
			ILess(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			void output(char buf[INSTR_BUF_SIZE]){
				return outputHelp(buf, "xor ", "xor ", ERROR_INSTR);
			}
	};

	class IAssn : public IBinOp {
	
	};

	// credits to godbolt!
	class IFlip : public IUnOp {
		public:
			IFlip(SubType _width, Label _src, Label _dest)
				   	: IUnOp(_width, _src, _dest) {
			}

			void output(char buf[INSTR_BUF_SIZE]){
				unsigned long long neg1 = OpUtils::genAllOne(type);
				if(neg1 == 0){
					return IXor(type, src, Label::genSInt(-1), dest).output(buf);
				} else {
					return IXor(type, src, neg1, dest).output(buf);
				}
			}
	};

	class INeg : public IUnOp {
		public:
			INeg(SubType _width, Label _src, Label _dest)
				   	: IUnOp(_width, _src, _dest) {
			}

			void output(char buf[INSTR_BUF_SIZE]){
				return ISub(type, Label::genSInt(0), src, dest).output(buf);
			}
	};

	// my next ssa is "s" plus this number.
	SSA nextSSA();
	// next label. Note, num is big enough 
	// that scoping shouldnt matter
	Label nextLabel();

	void genASM(Utils::Vector<IInstr>& buf);
}

#endif


