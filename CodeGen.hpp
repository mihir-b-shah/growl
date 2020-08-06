
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

    enum class SType:char {FLT_LIT, SIGN_LIT, USIGN_LIT, REF};
    
    /** Label is rep as L1 for instance */
    struct SSA {

        // passing 
        union {
            unsigned int ref;
            long long sint;
            unsigned long long uint;
            long double flt;
        } holder;
        SType which;

        SSA(){holder.ref = 0; which = SType::REF;}

        SSA(unsigned int _ref){
            holder.ref = _ref;
            which = SType::REF;
        }

        SSA operator++(int){
            if(__builtin_expect(which == SType::REF, true)){
                return SSA(holder.ref++);
            } else {
                Global::specifyError("SSA increment called on literal.\n", __FILE__, __LINE__);
                throw Global::DeveloperError;
            }
        }

        static SSA genUInt(unsigned long long v){
            SSA ref;
            ref.which = SType::USIGN_LIT;
            ref.holder.uint = v;
            return ref;
        }

        static SSA genSInt(long long v){
            SSA ref;
            ref.which = SType::SIGN_LIT;
            ref.holder.sint = v;
            return ref;
        }
        
        static SSA genFlt(long double v){
            SSA ref;
            ref.which = SType::FLT_LIT;
            ref.holder.flt = v;
            return ref;
        }

        bool isNull(){return which == SType::REF && holder.ref== 0;} 
        unsigned int extractLbl(){return holder.ref;}
        long long extractSignedInt(){return holder.sint;}
        unsigned long long extractUnsignedInt(){return holder.sint;}
        long double extractFlt(){return holder.flt;}
    };

    /* SSA is rep as %s1 for instance, good type safety too!*/
    struct Label {
        unsigned int lbl;

        Label(){lbl = 0;}

        Label(unsigned int _lbl){
            lbl = _lbl;
        }

        Label operator++(int){
            return Label(lbl++);
        }

        bool operator==(const Label& s2){
            return lbl == s2.lbl;
        }

        static Label nullLabel(){return Label(0);}
        unsigned int extract(){return lbl;}
    };

    static const unsigned int INSTR_BUF_SIZE = 100;
    static const char* const ERROR_INSTR = "NEVER USED. ERROR.";
    static const char* const EMPTY_INSTR = "";
    static const char* const EMPTY_FLAG = "";

    static const SubType EMPTY_TYPE = SubType::WHILE;
    
    struct OpUtils {
        static SType genSType(SubType type){
            switch(type){
                case SubType::CHAR:
                case SubType::LONG:
                case SubType::BOOL:
                    return SType::USIGN_LIT;
                case SubType::INT:
                    return SType::SIGN_LIT;
                case SubType::FLOAT:
                    return SType::FLT_LIT;
                case EMPTY_TYPE:
                    return SType::REF; // shouldnt matter.
                case SubType::VOID:
                default:
                    Global::specifyError("Type void or other passed to IR generator.\n", __FILE__, __LINE__);
                    throw Global::DeveloperError;
            }
        }

        static inline bool isUnsigned(SubType type){
            return SType::USIGN_LIT == genSType(type);
        }
        
        static inline bool isSigned(SubType type){
            return SType::SIGN_LIT == genSType(type);
        }

        static inline bool isFloat(SubType type){
            return SType::FLT_LIT == genSType(type);
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
                    Global::specifyError("Non integer type passed.\n", __FILE__, __LINE__);
                    throw Global::DeveloperError;
            }
        }

        static inline const char* formatSpec(SType lt){
            switch(lt){    
                case SType::FLT_LIT: 
                    return "%Lf";
                case SType::REF:
                    return "%%s%u";
                case SType::SIGN_LIT:
                    return "%lld";
                case SType::USIGN_LIT:
                    return "%llu";
            }
            return nullptr;
        }
    };    

    // inheritance hierarchy for all the different inclassions    
    class IInstr {
        public:
            virtual unsigned int output(char* buf) = 0;
    };

    class IBranch : public IInstr {
        private:
            SSA pred;
            Label ifbr;
            Label elsebr;
        public:
            IBranch(SSA _pred, Label _ifbr, Label _elsebr){
                pred = _pred;
                ifbr = _ifbr;
                elsebr = _elsebr;
            }

            unsigned int output(char* buf){
                if(pred.isNull()) { 
                    return std::snprintf(buf, INSTR_BUF_SIZE, "br L%u", ifbr.extract());
                } else {
                    switch(pred.which){
                        case SType::FLT_LIT:
                            Global::specifyError("Float passed to branch predicate.\n", __FILE__, __LINE__);
                            throw Global::InvalidBranch;
                        case SType::SIGN_LIT:
                            return std::snprintf(buf, INSTR_BUF_SIZE, "br L%u", 
                                    pred.extractSignedInt() != 0 ? ifbr.extract() : elsebr.extract());
                        case SType::USIGN_LIT:
                            return std::snprintf(buf, INSTR_BUF_SIZE, "br L%u", 
                                    pred.extractUnsignedInt() != 0 ? ifbr.extract() : elsebr.extract());
                        case SType::REF:
                            return std::snprintf(buf, INSTR_BUF_SIZE, "br i1 s%u, label L%u, label L%u",
                                    pred.extractLbl(), ifbr.extract(), elsebr.extract()); 
                    }
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
                         const char* const dType, bool sign, bool flt) = 0;

            unsigned int outputHelp(char* buf, const char* const iflg, 
                            const char* const fflg, const char* const unsignInstr, 
                            const char* const signInstr, const char* const fltInstr, 
                            bool dispFltType){
                
                assertTypeError(type);
                int ret;
                switch(type){
                    case SubType::CHAR:
                        ret = printOp(buf, unsignInstr, iflg, "i8 ", false, false);    
                        break;
                    case SubType::INT:
                        ret = printOp(buf, signInstr, iflg, "i32 ", true, false);
                        break;
                    case SubType::LONG:
                        ret = printOp(buf, unsignInstr, iflg, "i64 ", false, false);
                        break;
                    case SubType::BOOL:
                        ret = printOp(buf, unsignInstr, iflg, "i1 ", false, false);
                        break;
                    case SubType::FLOAT:
                        if(__builtin_expect(std::strcmp(fltInstr, ERROR_INSTR) == 0,false)){
                            Global::specifyError("Flt type used in int-only instruction.\n", __FILE__, __LINE__);    
                            throw Global::InvalidInstrInvocation;
                        }
                        // right now, floats in Growl are 64-bit doubles in LLVM.    
                        ret = printOp(buf, fltInstr, fflg, dispFltType ? "double " : "", true, true);
                        break;
                    case EMPTY_TYPE:
                        ret = printOp(buf, unsignInstr, iflg, "", false, false);
                        break;
                    default:
                        Global::specifyError("Invalid type encountered.\n", __FILE__, __LINE__);
                        throw Global::DeveloperError;
                }

                return ret;
            }    
    };

    class IBinOp : public IOp {
        private:
            enum:char {LIT_LIT_INT, LIT_LIT_FLT, LIT_REF_INT, LIT_REF_FLT, 
                        REF_LIT_INT, REF_LIT_FLT, REF_REF_INT, REF_REF_FLT};
        protected:
            SSA src1;
            SSA src2;
            SSA dest;

            IBinOp(SubType _width, SSA _src1, SSA _src2, SSA _dest) : IOp (_width) {
                src1 = _src1;
                src2 = _src2;
                dest = _dest;
            }

            void assertTypeError(SubType type) override {                
                // these are the unsigned types right now.
                SType corresp = OpUtils::genSType(type);
                if(__builtin_expect(dest.which != SType::REF && (src1.which != corresp 
                        || src1.which != SType::REF) && (src2.which != corresp 
                        || src2.which != SType::REF),false)){
                    Global::specifyError("Incorrect types passed to IR generator.\n", __FILE__, __LINE__);
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
                
                int chk;

                // checks if the instr is null, thats how ASSN is coded.
                chk = std::snprintf(fsbuf, FSBUF_LEN, "%%%%s%%u = %%s%%s%%s%s, %s",
                            spec1, spec2);
                if(chk >= FSBUF_LEN) {
                    Global::specifyError("Buffer for snprintf too small.\n", __FILE__, __LINE__);
                    throw Global::DeveloperError;
                }
               
                // example fsbuf later: "s%u = %s%s%s %lld %lld"

                switch(((src1.which == SType::REF) << 2) + ((src2.which == SType::REF) << 1) + flt){
                    case LIT_LIT_INT:
                        chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, flg, dType, 
                            sign ? src1.extractSignedInt() : src1.extractUnsignedInt(),
                            sign ? src2.extractSignedInt() : src2.extractUnsignedInt());
                        break;
                    case LIT_LIT_FLT:
                        chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, flg, dType, 
                            src1.extractFlt(), src2.extractFlt());
                        break;
                    case LIT_REF_INT:
                        chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, flg, dType, 
                            sign ? src1.extractSignedInt() : src1.extractUnsignedInt(), src2.extractLbl());
                        break;
                    case LIT_REF_FLT:
                        chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, flg, dType, 
                            sign ? src1.extractFlt() : src2.extractLbl());
                        break;
                    case REF_LIT_INT:
                        chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, flg, dType, 
                            src1.extractLbl(), sign ? src2.extractSignedInt() : src2.extractUnsignedInt());
                        break;
                    case REF_LIT_FLT:
                        chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, flg, dType, 
                            src1.extractLbl(), src2.extractFlt());
                        break;
                    case REF_REF_INT:
                    case REF_REF_FLT:
                        chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, flg, dType, 
                            src1.extractLbl(), src2.extractLbl());
                        break;
                    default:
                        Global::specifyError("Bool mask overflowed.\n", __FILE__, __LINE__);
                        throw Global::DeveloperError;            
                }

                if(chk >= INSTR_BUF_SIZE){
                    Global::specifyError("Buffer for snprintf too small.\n", __FILE__, __LINE__);
                    throw Global::DeveloperError;            
                }

                return chk;
            }
    };

    class IAdd : public IBinOp {
        public:
            IAdd(SubType _width, SSA _src1, SSA _src2, SSA _dest)
                      : IBinOp(_width, _src1, _src2, _dest) {
            }
            
            unsigned int output(char* buf){
                return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "add ", "add ", "fadd ", false);
            }
    };

    class ISub : public IBinOp {
        public:
            ISub(SubType _width,  SSA _src1, SSA _src2, SSA _dest)
                       : IBinOp(_width, _src1, _src2, _dest) {
            }

            unsigned int output(char* buf){
                return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "sub ", "sub ", "fsub ", false);    
            }
    };

    class IMul : public IBinOp {
        public:
            IMul(SubType _width, SSA _src1, SSA _src2, SSA _dest)
                       : IBinOp(_width, _src1, _src2, _dest) {
            }

            unsigned int output(char* buf){
                return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "mul ", "mul ", "fmul ", false);
            }
    };

    class IDiv : public IBinOp {
        public:
            IDiv(SubType _width, SSA _src1, SSA _src2, SSA _dest)
                       : IBinOp(_width, _src1, _src2, _dest) {
            }

            unsigned int output(char* buf){
                return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "udiv ", "sdiv ", "fdiv ", false);
            }
    };

    class IMod : public IBinOp {    
        public:
            IMod(SubType _width, SSA _src1, SSA _src2, SSA _dest)
                       : IBinOp(_width, _src1, _src2, _dest) {
            }

            unsigned int output(char* buf){
                return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "urem ", "srem ", "frem ", false);    
            }
    };

    class IShiftLeft : public IBinOp {
        public:
            IShiftLeft(SubType _width, SSA _src1, SSA _src2, SSA _dest)
                       : IBinOp(_width, _src1, _src2, _dest) {
            }

            unsigned int output(char* buf){
                return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "shl ", "shl ", ERROR_INSTR, false);    
            }
    };

    class IShiftRight : public IBinOp {
        public:
            IShiftRight(SubType _width, SSA _src1, SSA _src2, SSA _dest)
                       : IBinOp(_width, _src1, _src2, _dest) {
            }

            unsigned int output(char* buf){
                return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "lshr ", "ashr ", ERROR_INSTR, false);    
            }
    };

    class IAnd : public IBinOp {
        public:
            IAnd(SubType _width, SSA _src1, SSA _src2, SSA _dest)
                       : IBinOp(_width, _src1, _src2, _dest) {
            }

            unsigned int output(char* buf){
                return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "and ", "and ", ERROR_INSTR, false);
            }
    };

    class IOr : public IBinOp {
        public:
            IOr(SubType _width, SSA _src1, SSA _src2, SSA _dest)
                       : IBinOp(_width, _src1, _src2, _dest) {
            }

            unsigned int output(char* buf){
                return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "or ", "or ", ERROR_INSTR, false);
            }
    };

    class IXor : public IBinOp {
        public:
            IXor(SubType _width, SSA _src1, SSA _src2, SSA _dest)
                       : IBinOp(_width, _src1, _src2, _dest) {
            }

            unsigned int output(char* buf){
                return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "xor ", "xor ", ERROR_INSTR, false);
            }
    };

    class IUnOp : public IOp {
        private:
            enum:char {LIT_INT, LIT_FLT, REF_INT, REF_FLT};
        protected:
            SSA src;
            SSA dest;
    
            IUnOp(SubType _width, SSA _src, SSA _dest) : IOp(_width) {
                src = _src;
                dest = _dest;
            }

            void assertTypeError(SubType type) override {                
                // these are the unsigned types right now.
                SType corresp = OpUtils::genSType(type);
                if(__builtin_expect(dest.which != SType::REF && (src.which != corresp 
                        || src.which != SType::REF) ,false)){
                    Global::specifyError("Incorrect types passed to IR generator.\n", __FILE__, __LINE__);
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
                
                int chk;
                
                // check if instr is null (assn case)
                if(strlen(instr) == 0){
                    chk = std::snprintf(fsbuf, FSBUF_LEN, "%%%%s%%u = %%s%%s%%s%s", spec);
                } else {
                    chk = std::snprintf(fsbuf, FSBUF_LEN, "%%%%s%%u = %%s%%s%%s %s", spec);
                }
                
                if(chk >= FSBUF_LEN) {
                    Global::specifyError("Buffer for snprintf too small.\n", __FILE__, __LINE__);
                    throw Global::DeveloperError;
                }
               
                // example fsbuf later: "s%u = %s%s%s %lld %lld"
                
                switch(((src.which == SType::REF) << 1) + flt){
                    case LIT_INT:
                        chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, flg, dType, 
                            sign ? src.extractSignedInt() : src.extractUnsignedInt());
                        break;
                    case LIT_FLT:
                        chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, flg, dType, 
                            src.extractFlt());
                        break;
                    case REF_INT:
                    case REF_FLT:
                        chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest.extractLbl(), instr, flg, dType, 
                            src.extractLbl());
                        break;
                    default:
                        Global::specifyError("Bool mask overflowed.\n", __FILE__, __LINE__);
                        throw Global::DeveloperError;            
                }

                if(chk >= INSTR_BUF_SIZE){
                    Global::specifyError("Buffer for snprintf too small.\n", __FILE__, __LINE__);
                    throw Global::DeveloperError;            
                }

                return chk;
            }
    };

    // not done yet
    class IGreater : public IBinOp {
        public:
            IGreater(SubType _width, SSA _src1, SSA _src2, SSA _dest)
                       : IBinOp(_width, _src1, _src2, _dest) {
            }

            /** This uses the "ugt" instruction, which means floats can be NaN or special
             * values. Optimize later to get to use "ogt" which is much faster */
            unsigned int output(char* buf){
                return outputHelp(buf, OpUtils::isUnsigned(type) ? "ugt " : "sgt ", 
                                "ugt ", "icmp ", "icmp ", "fcmp ", true);
            }
    };

    class ILess : public IBinOp {
        public:
            ILess(SubType _width, SSA _src1, SSA _src2, SSA _dest)
                       : IBinOp(_width, _src1, _src2, _dest) {
            }

            unsigned int output(char* buf){
                return outputHelp(buf, OpUtils::isUnsigned(type) ? "ult " : "slt ", 
                                "ult ", "icmp ", "icmp ", "fcmp ", true);
            }
    };

    class IEqual : public IBinOp {
        public:
            IEqual(SubType _width, SSA _src1, SSA _src2, SSA _dest)
                       : IBinOp(_width, _src1, _src2, _dest) {
            }

            unsigned int output(char* buf){
                return outputHelp(buf, OpUtils::isUnsigned(type) ? "eq " : "eq ", 
                                "ueq ", "icmp ", "icmp ", "fcmp ", true);
            }
    };

    /** For use in IAssn */
    class IDummy : public IUnOp {
        public:
            IDummy(SSA _src, SSA _dest) 
                    // note, WHILE is a placeholder for nothing.
                    // see the code in IUnOp
                    : IUnOp(EMPTY_TYPE, _src, _dest) {
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
            IAssn(SubType _width, SSA _src1, SSA _src2, SSA _dest)
                       : IBinOp(_width, _src1, _src2, _dest) {
            }

            /** Should still fit in INSTR_BUF_SIZE-1 */
            unsigned int output(char* buf){
                unsigned int len1 = IDummy(src1, src2).output(buf);
                buf[len1++] = '\n';
                return len1+IDummy(src2, dest).output(buf+len1);
            }
    };

    // credits to godbolt!
    class IFlip : public IUnOp {
        public:
            IFlip(SubType _width, SSA _src, SSA _dest)
                       : IUnOp(_width, _src, _dest) {
            }

            unsigned int output(char* buf){
                unsigned long long neg1 = OpUtils::genAllOne(type);
                if(neg1 == 0){
                    return IXor(type, src, SSA::genSInt(-1), dest).output(buf);
                } else {
                    return IXor(type, src, neg1, dest).output(buf);
                }
            }
    };

    class INeg : public IUnOp {
        public:
            INeg(SubType _width, SSA _src, SSA _dest)
                       : IUnOp(_width, _src, _dest) {
            }

            unsigned int output(char* buf){
                return ISub(type, SSA::genSInt(0), src, dest).output(buf);
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
