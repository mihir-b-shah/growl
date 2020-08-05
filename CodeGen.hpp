
#ifndef CODE_GEN_HPP
#define CODE_GEN_HPP

#include "Lex.h"
#include "Vector.hpp"
#include "Error.h"
#include <cstdio>
#include <cstring>
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

	static const unsigned int INSTR_BUF_SIZE = 100;
	static const char* const ERROR_INSTR = "NEVER USED. ERROR.";
	static const char* const EMPTY_INSTR = "";
	static const char* const EMPTY_FLAG = "";
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

		static inline bool isUnsigned(SubType type){
			return LType::USIGN_LIT == genLType(type);
		}
		
		static inline bool isSigned(SubType type){
			return LType::SIGN_LIT == genLType(type);
		}

		static inline bool isFloat(SubType type){
			return LType::USIGN_LIT == genLType(type);
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
					return "%%Lf";
				case LType::LBL:
					return "s%%%%u";
				case LType::SIGN_LIT:
					return "%%lld";
				case LType::USIGN_LIT:
					return "%%llu";
			}
		}
	};	

	// inheritance hierarchy for all the different inclassions	
	class IInstr {
		virtual unsigned int output(char* buf) = 0;
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

		unsigned int output(char* buf){
			if(pred == SSA::nullSSA()) { 
				return std::snprintf(buf, INSTR_BUF_SIZE, "br L%u", ifbr.extractLbl());
			} else {
				return std::snprintf(buf, INSTR_BUF_SIZE, "br i1 s%u, label L%u, label L%u",
								pred.extract(), ifbr.extractLbl(), elsebr.extractLbl()); 
			}
		}	
	};

	class IOp : public IInstr {
		protected:
			SubType type;
			IOp(SubType _type){
				type = _type;
			}

			virtual void assertTypeError(SubType type) = 0;
			virtual unsigned int printOp(char* buf, const char* const instr, const char* const flg,
						 const char* const dType, bool sign, bool flt);

			unsigned int outputHelp(char* buf, const char* const iflg, 
							const char* const fflg, const char* const unsignInstr, 
							const char* const signInstr, const char* const fltInstr, 
							bool dispFltType){
				assertTypeError(type);
				int ret;
				switch(type){
					case SubType::CHAR:
						ret = printOp(buf, unsignInstr, iflg, "i8", false, false);	
						break;
					case SubType::INT:
						ret = printOp(buf, signInstr, iflg, "i32", true, false);
						break;
					case SubType::LONG:
						ret = printOp(buf, unsignInstr, iflg, "i64", false, false);
						break;
					case SubType::BOOL:
						ret = printOp(buf, unsignInstr, iflg, "i1", false, false);
						break;
					case SubType::FLOAT:
						if(__builtin_expect(std::strcmp(fltInstr, ERROR_INSTR) == 0,false)){
							Global::specifyError("Flt type used in int-only instruction.\n");	
							throw Global::InvalidInstrInvocation;
						}
						// right now, floats in Growl are 64-bit doubles in LLVM.	
						ret = printOp(buf, fltInstr, fflg, dispFltType ? "double" : "", true, true);
						break;
					default:
						Global::specifyError("Invalid type encountered.\n");
						throw Global::DeveloperError;
				}

				return ret;
			}	
		public:
			virtual unsigned int output(char* buf) = 0;
	};

	class IBinOp : public IOp {
		private:
			enum:char {LIT_LIT_INT, LIT_LIT_FLT, LIT_LBL_INT, LIT_LBL_FLT, 
						LBL_LIT_INT, LBL_LIT_FLT, LBL_LBL_INT, LBL_LBL_FLT};
		protected:
			Label src1;
			Label src2;
			Label dest;

			IBinOp(SubType _width, Label _src1, Label _src2, Label _dest) : IOp (_width) {
				src1 = _src1;
				src2 = _src2;
				dest = _dest;
			}

			void assertTypeError(SubType type) override {				
				// these are the unsigned types right now.
				LType corresp = OpUtils::genLType(type);
				if(__builtin_expect(dest.which != LType::LBL && (src1.which != corresp 
						|| src1.which != LType::LBL) && (src2.which != corresp 
						|| src2.which != LType::LBL),false)){
					Global::specifyError("Incorrect types passed to IR generator.\n");
					throw Global::DeveloperError;
				}
			}

			/** "instr" and :"flg" should be followed by space.*/
			unsigned int printOp(char* buf, const char* const instr, const char* const flg,
						 const char* const dType, bool sign, bool flt) override {
				
				const char* spec1 = OpUtils::formatSpec(src1.which);
				const char* spec2 = OpUtils::formatSpec(src2.which);

				// the max number of chars it can be.
				constexpr int FSBUF_LEN = 30;
				
				char fsbuf[FSBUF_LEN] = {'\0'};
				// setup the printf. bad for efficiency but whatever...
				int chk = std::snprintf(fsbuf, FSBUF_LEN, "%%%%s%%u = %%s%%s%%s %s %s",
								spec1, spec2);
				
				if(chk >= FSBUF_LEN) {
					Global::specifyError("Buffer for snprintf too small.\n");
					throw Global::DeveloperError;
				}
				
				// example fsbuf later: "s%u = %s%s%s %lld %lld"

				switch(((src1.which == LType::LBL) << 2) + ((src2.which == LType::LBL) << 1) + flt){
					case LIT_LIT_INT:
						chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, flg, dType, 
							sign ? src1.extractSignedInt() : src1.extractUnsignedInt(),
							sign ? src2.extractSignedInt() : src2.extractUnsignedInt());
						break;
					case LIT_LIT_FLT:
						chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, flg, dType, 
							src1.extractFlt(), src2.extractFlt());
						break;
					case LIT_LBL_INT:
						chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, flg, dType, 
							sign ? src1.extractSignedInt() : src1.extractUnsignedInt(), src2.extractLbl());
						break;
					case LIT_LBL_FLT:
						chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, flg, dType, 
							sign ? src1.extractFlt() : src2.extractLbl());
						break;
					case LBL_LIT_INT:
						chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, flg, dType, 
							src1.extractLbl(), sign ? src2.extractSignedInt() : src2.extractUnsignedInt());
						break;
					case LBL_LIT_FLT:
						chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, flg, dType, 
							src1.extractLbl(), src2.extractFlt());
						break;
					case LBL_LBL_INT:
					case LBL_LBL_FLT:
						chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, flg, dType, 
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

				return chk;
			}
	};

	class IAdd : public IBinOp {
		public:
			IAdd(SubType _width, Label _src1, Label _src2, Label _dest)
				  	: IBinOp(_width, _src1, _src2, _dest) {
			}
			
			unsigned int output(char* buf){
				return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "add ", "add ", "fadd ", false);
			}
	};

	class ISub : public IBinOp {
		public:
			ISub(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			unsigned int output(char* buf){
				return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "sub ", "sub ", "fsub ", false);	
			}
	};

	class IMul : public IBinOp {
		public:
			IMul(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			unsigned int output(char* buf){
				return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "mul ", "mul ", "fmul ", false);
			}
	};

	class IDiv : public IBinOp {
		public:
			IDiv(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			unsigned int output(char* buf){
				return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "udiv ", "sdiv ", "fdiv ", false);
			}
	};

	class IMod : public IBinOp {	
		public:
			IMod(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			unsigned int output(char* buf){
				return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "urem ", "srem ", "frem ", false);	
			}
	};

	class IShiftLeft : public IBinOp {
		public:
			IShiftLeft(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			unsigned int output(char* buf){
				return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "shl ", "shl ", ERROR_INSTR, false);	
			}
	};

	class IShiftRight : public IBinOp {
		public:
			IShiftRight(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			unsigned int output(char* buf){
				return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "lshr ", "ashr ", ERROR_INSTR, false);	
			}
	};

	class IAnd : public IBinOp {
		public:
			IAnd(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			unsigned int output(char* buf){
				return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "and ", "and ", ERROR_INSTR, false);
			}
	};

	class IOr : public IBinOp {
		public:
			IOr(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			unsigned int output(char* buf){
				return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "or ", "or ", ERROR_INSTR, false);
			}
	};

	class IXor : public IBinOp {
		public:
			IXor(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			unsigned int output(char* buf){
				return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "xor ", "xor ", ERROR_INSTR, false);
			}
	};

	class IUnOp : public IOp {
		private:
			enum:char {LIT_INT, LIT_FLT, LBL_INT, LBL_FLT};
		protected:
			Label src;
			Label dest;
	
			IUnOp(SubType _width, Label _src, Label _dest) : IOp(_width) {
				src = _src;
				dest = _dest;
			}

			void assertTypeError(SubType type) override {				
				// these are the unsigned types right now.
				LType corresp = OpUtils::genLType(type);
				if(__builtin_expect(dest.which != LType::LBL && (src.which != corresp 
						|| src.which != LType::LBL) ,false)){
					Global::specifyError("Incorrect types passed to IR generator.\n");
					throw Global::DeveloperError;
				}
			}

			/** "instr" and :"flg" should be followed by space.*/
			unsigned int printOp(char* buf, const char* const instr, const char* const flg,
						 const char* const dType, bool sign, bool flt) override {
				
				const char* spec = OpUtils::formatSpec(src.which);

				// the max number of chars it can be.
				constexpr int FSBUF_LEN = 30;
				
				char fsbuf[FSBUF_LEN] = {'\0'};
				// setup the printf. bad for efficiency but whatever...
				int chk = std::snprintf(fsbuf, FSBUF_LEN, "%%%%s%%u = %%s%%s%%s %s", spec);
				
				if(chk >= FSBUF_LEN) {
					Global::specifyError("Buffer for snprintf too small.\n");
					throw Global::DeveloperError;
				}
				
				// example fsbuf later: "s%u = %s%s%s %lld %lld"

				switch(((src.which == LType::LBL) << 2) + flt){
					case LIT_INT:
						chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, flg, dType, 
							sign ? src.extractSignedInt() : src.extractUnsignedInt());
						break;
					case LIT_FLT:
						chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, flg, dType, 
							src.extractFlt());
						break;
					case LBL_INT:
					case LBL_FLT:
						chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, flg, dType, 
							src.extractLbl());
						break;
					default:
						Global::specifyError("Bool mask overflowed.\n");
						throw Global::DeveloperError;			
				}

				if(chk >= INSTR_BUF_SIZE){
					Global::specifyError("Buffer for snprintf too small.\n");
					throw Global::DeveloperError;			
				}

				return chk;
			}
	};

	// not done yet
	class IGreater : public IBinOp {
		public:
			IGreater(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			/** This uses the "ugt" instruction, which means floats can be NaN or special
			 * values. Optimize later to get to use "ogt" which is much faster */
			unsigned int output(char* buf){
				return outputHelp(buf, OpUtils::isUnsigned(type) ? "ugt" : "sgt", 
								"ugt ", "icmp ", "icmp ", "fcmp ", true);
			}
	};

	class ILess : public IBinOp {
		public:
			ILess(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			unsigned int output(char* buf){
				return outputHelp(buf, OpUtils::isUnsigned(type) ? "ult" : "slt", 
								"ult ", "icmp ", "icmp ", "fcmp ", true);
			}
	};

	class IEqual : public IBinOp {
		public:
			IEqual(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			unsigned int output(char* buf){
				return outputHelp(buf, OpUtils::isUnsigned(type) ? "ult" : "slt", 
								"ueq ", "icmp ", "icmp ", "fcmp ", true);
			}
	};

	/** For use in IAssn */
	class IDummy : public IUnOp {
		public:
			IDummy(Label _src, Label _dest) 
					: IUnOp(SubType::CHAR, _src, _dest) {
			}

			unsigned int output(char* buf) {
				return outputHelp(buf, "", "", "", "", "", false);
			}
	};

	/** 
	 * This is slightly peculiar.
	 * We use this:
	 *
	 * Suppose high level code is y = x.
	 * Turns into %y = %x
	 * Followed by %z = %y. %z is the exported value.
	 */
	class IAssn : public IBinOp {
	
		public:
			IAssn(SubType _width, Label _src1, Label _src2, Label _dest)
				   	: IBinOp(_width, _src1, _src2, _dest) {
			}

			/** Should still fit in INSTR_BUF_SIZE-1 */
			unsigned int output(char* buf){
				unsigned int len1 = IDummy(src1, src2).output(buf);
				buf[len1++] = '\n';
				return len1+IDummy(src2, dest).output(buf);
			}
	};

	// credits to godbolt!
	class IFlip : public IUnOp {
		public:
			IFlip(SubType _width, Label _src, Label _dest)
				   	: IUnOp(_width, _src, _dest) {
			}

			unsigned int output(char* buf){
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

			unsigned int output(char* buf){
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


