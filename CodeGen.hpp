
#ifndef CODE_GEN_HPP
#define CODE_GEN_HPP

#include "Parse.h"
#include "Vector.hpp"
#include "Error.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <utility>

namespace CodeGen {

    enum class SType:char {FLT_LIT, SIGN_LIT, USIGN_LIT, REF};
    
    /** Label is rep as L1 for instance */
    struct SSA {

        // passing 
        union {
            unsigned int ref;
            long long sint;
            unsigned long long uint;
            double flt;
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
        
        static SSA genFlt(double v){
            SSA ref;
            ref.which = SType::FLT_LIT;
            ref.holder.flt = v;
            return ref;
        }

        static SSA nullValue(){return SSA();}
        bool isNull(){return which == SType::REF && holder.ref== 0;} 
        
        unsigned int extractLbl(){return holder.ref;}
        long long extractSignedInt(){return holder.sint;}
        unsigned long long extractUnsignedInt(){return holder.sint;}
        double extractFlt(){return holder.flt;}
    };

    /* SSA is rep as %s1 for instance, good type safety too!*/
    class IInstr;
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

    SSA nextSSA();
    // next label. Note, num is big enough 
    // that scoping shouldnt matter
    Label nextLabel();
    
    // just 12 bytes 
    void insertVarSSA(unsigned int Var_Extr, SSA ssa);
    SSA getFromVar(unsigned int Var_Extract);

    Label getFromAST(unsigned int AST_Extract);
    void insertASTLbl(unsigned int AST_Extr, Label lbl);
    static const unsigned int INSTR_BUF_SIZE = 100;
    static const char* const ERROR_INSTR = "NEVER USED. ERROR.";
    static const char* const EMPTY_INSTR = "";
    static const char* const EMPTY_FLAG = "";

    using Parse::IntrOps;
    using Parse::VarType;
    
    static const IntrOps EMPTY_OP = IntrOps::OTHER;
    static const VarType EMPTY_TYPE = VarType::OTHER;

    struct OpUtils {
        static SType genSType(VarType type){
            switch(type){
                case VarType::CHAR:
                case VarType::LONG:
                case VarType::BOOL:
                    return SType::USIGN_LIT;
                case VarType::INT:
                    return SType::SIGN_LIT;
                case VarType::FLOAT:
                    return SType::FLT_LIT;
                case VarType::OTHER:
                    return SType::REF;
                case VarType::VOID:
                default:
                    Global::specifyError("Type void or other passed to IR generator.\n", __FILE__, __LINE__);
                    throw Global::DeveloperError;
            }
        }

        static inline bool isUnsigned(VarType type){
            return SType::USIGN_LIT == genSType(type);
        }
        
        static inline bool isSigned(VarType type){
            return SType::SIGN_LIT == genSType(type);
        }

        static inline bool isFloat(VarType type){
            return SType::FLT_LIT == genSType(type);
        }

        /** Returns 0 if should be signed */
        static unsigned long long genAllOne(VarType type){
            switch(type){
                case VarType::CHAR:
                    return 0xff;
                case VarType::LONG:
                    return 0xffffffffffffffffULL;
                case VarType::BOOL:
                    return 1;
                case VarType::INT:
                    return 0;
                case VarType::FLOAT:
                case VarType::VOID:
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

    enum class LLVMInstr:char {_Br, _Add, _Sub, _Mul, _Div, _Mod, _Shl, _Shr, 
            _And, _Or, _Xor, _Gr, _Ls, _Eq, _Dmy, _Asn, _Flp, _Neg, _Decl, _Cast};

    static constexpr LLVMInstr unusedLLVMInstr(){
        return LLVMInstr::_Dmy;
    }

    enum class UOB:char {UNARY, BINARY, OTHER};

    // inheritance hierarchy for all the different instructions
    // same situation as ArgIterator, a polymorphic value type. 
    class IInstr {
        friend class IRProg;
        private:
            enum BinEnum:char {LIT_LIT_INT, LIT_LIT_FLT, LIT_REF_INT, LIT_REF_FLT, 
                        REF_LIT_INT, REF_LIT_FLT, REF_REF_INT, REF_REF_FLT};
            enum UnEnum:char {LIT_INT, LIT_FLT, REF_INT, REF_FLT};

            LLVMInstr _instr;

            union Hold {
                struct {
                    SSA _pred;
                    Label _ifbr;
                    Label _elsebr;
                } br;
                struct {
                    VarType _type;
                    SSA _src1;
                    SSA _src2;
                    SSA _dest;
                } opr;
                struct {
                    VarType _inType;
                    VarType _outType;
                    SSA _src;
                    SSA _dest;
                } cast;
                
                Hold(){}
            } bop;
            
            inline LLVMInstr& instr() {return _instr;} 
            inline SSA& pred(){return bop.br._pred;}
            inline Label& ifbr(){return bop.br._ifbr;}
            inline Label& elsebr(){return bop.br._elsebr;}

            inline VarType& type(){return bop.opr._type;}
            inline SSA& src1(){return bop.opr._src1;}
            inline SSA& src2(){return bop.opr._src2;}
            inline SSA& dest(){return bop.opr._dest;}

            inline VarType& iType(){return bop.cast._inType;}
            inline VarType& oType(){return bop.cast._outType;}
            inline SSA& cSrc(){return bop.cast._src;}
            inline SSA& cDest(){return bop.cast._dest;}

            void assertTypeErrorBinary(VarType type) {                
                // these are the unsigned types right now.
                SType corresp = OpUtils::genSType(type);
                if(__builtin_expect(dest().which != SType::REF && (src1().which != corresp 
                        || src1().which != SType::REF) && (src2().which != corresp 
                        || src2().which != SType::REF),false)){
                    Global::specifyError("Incorrect types passed to IR generator.\n", __FILE__, __LINE__);
                    throw Global::DeveloperError;
                }
            }

            unsigned int printOpBinary(char* buf, const char* const instr, const char* const flg,
                         const char* const dType, bool sign, bool flt) {
                
                const char* spec1 = OpUtils::formatSpec(src1().which);
                const char* spec2 = OpUtils::formatSpec(src2().which);

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

                switch(((src1().which == SType::REF) << 2) + ((src2().which == SType::REF) << 1) + flt){
                    case LIT_LIT_INT:
                        chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest().extractLbl(), instr, flg, dType, 
                            sign ? src1().extractSignedInt() : src1().extractUnsignedInt(),
                            sign ? src2().extractSignedInt() : src2().extractUnsignedInt());
                        break;
                    case LIT_LIT_FLT:
                        chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest().extractLbl(), instr, flg, dType, 
                            src1().extractFlt(), src2().extractFlt());
                        break;
                    case LIT_REF_INT:
                        chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest().extractLbl(), instr, flg, dType, 
                            sign ? src1().extractSignedInt() : src1().extractUnsignedInt(), src2().extractLbl());
                        break;
                    case LIT_REF_FLT:
                        chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest().extractLbl(), instr, flg, dType, 
                            sign ? src1().extractFlt() : src2().extractLbl());
                        break;
                    case REF_LIT_INT:
                        chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest().extractLbl(), instr, flg, dType, 
                            src1().extractLbl(), sign ? src2().extractSignedInt() : src2().extractUnsignedInt());
                        break;
                    case REF_LIT_FLT:
                        chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest().extractLbl(), instr, flg, dType, 
                            src1().extractLbl(), src2().extractFlt());
                        break;
                    case REF_REF_INT:
                    case REF_REF_FLT:
                        chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest().extractLbl(), instr, flg, dType, 
                            src1().extractLbl(), src2().extractLbl());
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
            void assertTypeErrorUnary(VarType type) {                
                // these are the unsigned types right now.
                SType corresp = OpUtils::genSType(type);
                if(__builtin_expect(dest().which != SType::REF && (src1().which != corresp 
                        || src1().which != SType::REF), false)){
                    Global::specifyError("Incorrect types passed to IR generator.\n", __FILE__, __LINE__);
                    throw Global::DeveloperError;
                }
            }

            bool needsNoSpace(){
                return instr() == LLVMInstr::_Dmy || instr() == LLVMInstr::_Decl;
            }

            /** "instr" and :"flg" should be followed by space.*/
            unsigned int printOpUnary(char* buf, const char* const iinstr, const char* const flg,
                         const char* const dType, bool sign, bool flt) {
                
                const char* spec = OpUtils::formatSpec(src1().which);

                // the max number of chars it can be.
                constexpr int FSBUF_LEN = 30;
                
                char fsbuf[FSBUF_LEN] = {'\0'};
                // setup the printf. bad for efficiency but whatever...
                
                int chk;
                
                // check if instr is null (assn case)
                if(needsNoSpace()){
                    chk = std::snprintf(fsbuf, FSBUF_LEN, "%%%%s%%u = %%s%%s%%s%s", spec);
                } else {
                    chk = std::snprintf(fsbuf, FSBUF_LEN, "%%%%s%%u = %%s%%s%%s %s", spec);
                }
                
                if(chk >= FSBUF_LEN) {
                    Global::specifyError("Buffer for snprintf too small.\n", __FILE__, __LINE__);
                    throw Global::DeveloperError;
                }
              
                // example fsbuf later: "s%u = %s%s%s %lld %lld"
                std::cout << "Swcode: " << (((src1().which == SType::REF) << 1) + flt) << '\n'; 
                switch(((src1().which == SType::REF) << 1) + flt){
                    case LIT_INT:
                        chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest().extractLbl(), iinstr, flg, dType, 
                            sign ? src1().extractSignedInt() : src1().extractUnsignedInt());
                        break;
                    case LIT_FLT:
                        chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest().extractLbl(), iinstr, flg, dType, 
                            src1().extractFlt());
                        break;
                    case REF_INT:
                    case REF_FLT:
                        chk = std::snprintf(buf, INSTR_BUF_SIZE, fsbuf, dest().extractLbl(), iinstr, flg, dType, 
                            src1().extractLbl());
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
            
            static constexpr unsigned ord(VarType type){
                return static_cast<unsigned>(type);
            }

            static constexpr unsigned gCv(VarType in, VarType out){
                constexpr unsigned NUM_TYPES = 5;
                return ord(in) + NUM_TYPES*ord(out);
            }

            inline unsigned int printCastHelp(char* buf, const char* const instr){
                return std::snprintf(buf, INSTR_BUF_SIZE, instr, 
                                cDest().extractLbl(), cSrc().extractLbl());
            }
            
            // fill in the casts.
            unsigned int castFmt(char* buf){
                constexpr unsigned NUM_TYPES = 5;
                switch(ord(iType()) + NUM_TYPES*ord(oType())){
                    case gCv(VarType::BOOL, VarType::CHAR):
                        return printCastHelp(buf, "%%s%u = zext i1 %%s%u to i8");
                    case gCv(VarType::BOOL, VarType::INT):
                        return printCastHelp(buf, "%%s%u = zext i1 %%s%u to i32");
                    case gCv(VarType::BOOL, VarType::LONG):
                        return printCastHelp(buf, "%%s%u = zext i1 %%s%u to i64");
                    case gCv(VarType::BOOL, VarType::FLOAT):
                        return printCastHelp(buf, "%%s%u = uitofp i1 %%s%u to double"); // right now only f64 supported.
                    case gCv(VarType::CHAR, VarType::BOOL):
                        return printCastHelp(buf, "%%s%u = icmp ne i8 %%s%u 0");
                    case gCv(VarType::CHAR, VarType::INT):
                        return printCastHelp(buf, "%%s%u = zext i8 %%s%u to i32");
                    case gCv(VarType::CHAR, VarType::LONG):
                        return printCastHelp(buf, "%%s%u = zext i8 %%s%u to i32");
                    case gCv(VarType::CHAR, VarType::FLOAT):
                        return printCastHelp(buf, "%%s%u = uitofp i8 %%s%u to double");
                    case gCv(VarType::INT, VarType::BOOL):
                        return printCastHelp(buf, "%%s%u = icmp ne i32 %%s%u 0");
                    case gCv(VarType::INT, VarType::CHAR):
                        return printCastHelp(buf, "%%s%u = trunc i32 %%s%u to i8");
                    case gCv(VarType::INT, VarType::LONG):
                        return printCastHelp(buf, "%%s%u = zext i32 %%s%u to i64");
                    case gCv(VarType::INT, VarType::FLOAT):
                        return printCastHelp(buf, "%%s%u = uitofp i32 %%s%u to double");
                    case gCv(VarType::LONG, VarType::BOOL):
                        return printCastHelp(buf, "%%s%u = icmp ne i64 %%s%u 0");
                    case gCv(VarType::LONG, VarType::CHAR):
                        return printCastHelp(buf, "%%s%u = trunc i64 %%s%u to i8");
                    case gCv(VarType::LONG, VarType::INT):
                    {
                        SSA temp = CodeGen::nextSSA().extractLbl();
                        unsigned adv = std::snprintf(buf, INSTR_BUF_SIZE, "%%s%u = and i64 %%s%u, %x", 
                                cSrc().extractLbl(), temp.extractLbl(), 0x7fff'ffff);
                        return adv + std::snprintf(buf+adv, INSTR_BUF_SIZE, "%%s%u = trunc i64 %%s%u to i32",
                                        temp.extractLbl(), cDest().extractLbl());
                    }
                    case gCv(VarType::LONG, VarType::FLOAT):
                        return printCastHelp(buf, "%%s%u = uitofp i64 %%s%u to double");
                    case gCv(VarType::FLOAT, VarType::BOOL):
                        return printCastHelp(buf, "%%s%u = fptoui double %%s%u to i1");
                    case gCv(VarType::FLOAT, VarType::CHAR):
                        return printCastHelp(buf, "%%s%u = fptoui double %%s%u to i8");
                    case gCv(VarType::FLOAT, VarType::INT):
                        return printCastHelp(buf, "%%s%u = fptosi double %%s%u to i32");
                    case gCv(VarType::FLOAT, VarType::LONG):
                        return printCastHelp(buf, "%%s%u = fptoui double %%s%u to i64");
                }
                return 0;
            }

            unsigned int assnOutput(char* buf){
                if(__builtin_expect(dest().which != SType::REF, false)){
                    Global::specifyError("Assignment to nonvar",
                                    __FILE__, __LINE__);
                    throw Global::DeveloperError;
                }
                switch(src1().which){
                    case SType::REF:
                        return std::snprintf(buf, INSTR_BUF_SIZE, "%%s%u = %%s%u",
                                        dest().extractLbl(), src1().extractLbl()); 
                    case SType::FLT_LIT:
                        return std::snprintf(buf, INSTR_BUF_SIZE, "%%s%u = %f",
                                        dest().extractLbl(), src1().extractFlt()); 
                    case SType::SIGN_LIT:
                        return std::snprintf(buf, INSTR_BUF_SIZE, "%%s%u = %lld",
                                        dest().extractLbl(), src1().extractSignedInt()); 
                    case SType::USIGN_LIT:
                        return std::snprintf(buf, INSTR_BUF_SIZE, "%%s%u = %llu",
                                        dest().extractLbl(), src1().extractUnsignedInt()); 
                }
                return 0; 
            }

            unsigned int brOutput(char* buf) {
                if(pred().isNull()) { 
                    return std::snprintf(buf, INSTR_BUF_SIZE, "br L%u", ifbr().extract());
                } else {
                    switch(pred().which){
                        case SType::FLT_LIT:
                            Global::specifyError("Float passed to branch predicate.\n", __FILE__, __LINE__);
                            throw Global::InvalidBranch;
                        case SType::SIGN_LIT:
                            return std::snprintf(buf, INSTR_BUF_SIZE, "br L%u", 
                                    pred().extractSignedInt() != 0 ? ifbr().extract() : elsebr().extract());
                        case SType::USIGN_LIT:
                            return std::snprintf(buf, INSTR_BUF_SIZE, "br L%u", 
                                    pred().extractUnsignedInt() != 0 ? ifbr().extract() : elsebr().extract());
                        case SType::REF:
                            return std::snprintf(buf, INSTR_BUF_SIZE, "br i1 s%u, label L%u, label L%u",
                                    pred().extractLbl(), ifbr().extract(), elsebr().extract()); 
                    }
                }
                return 0;
            }

            UOB getUOB(){
                switch(instr()){
                    case LLVMInstr::_Add:
                    case LLVMInstr::_Sub:
                    case LLVMInstr::_Mul:
                    case LLVMInstr::_Div: 
                    case LLVMInstr::_Mod:
                    case LLVMInstr::_Shl:
                    case LLVMInstr::_Shr:
                    case LLVMInstr::_And:
                    case LLVMInstr::_Or:
                    case LLVMInstr::_Xor:
                    case LLVMInstr::_Gr:
                    case LLVMInstr::_Ls:
                    case LLVMInstr::_Eq:
                    case LLVMInstr::_Asn:
                        return UOB::BINARY;
                    case LLVMInstr::_Flp:
                    case LLVMInstr::_Neg:
                    case LLVMInstr::_Dmy:
                    case LLVMInstr::_Decl:
                    case LLVMInstr::_Cast:
                        return UOB::UNARY;
                    case LLVMInstr::_Br:
                        return UOB::OTHER;
                }
                return UOB::OTHER;
            }

            unsigned int outputHelp(char* buf, const char* const iflg, 
                            const char* const fflg, const char* const unsignInstr, 
                            const char* const signInstr, const char* const fltInstr, 
                            bool dispFltType){
                
                switch(getUOB()){
                    case UOB::UNARY:
                    {
                        assertTypeErrorUnary(type());
                        int ret;
                        switch(type()){
                            case VarType::CHAR:
                                ret = printOpUnary(buf, unsignInstr, iflg, "i8 ", false, false);    
                                break;
                            case VarType::INT:
                                ret = printOpUnary(buf, signInstr, iflg, "i32 ", true, false);
                                break;
                            case VarType::LONG:
                                ret = printOpUnary(buf, unsignInstr, iflg, "i64 ", false, false);
                                break;
                            case VarType::BOOL:
                                ret = printOpUnary(buf, unsignInstr, iflg, "i1 ", false, false);
                                break;
                            case VarType::FLOAT:
                                if(__builtin_expect(std::strcmp(fltInstr, ERROR_INSTR) == 0,false)){
                                    Global::specifyError("Flt type used in int-only instruction.\n", __FILE__, __LINE__);    
                                    throw Global::InvalidInstrInvocation;
                                }
                                // right now, floats in Growl are 64-bit floats in LLVM.
                                ret = printOpUnary(buf, fltInstr, fflg, dispFltType ? "double " : "", true, true);
                                break;
                            case VarType::OTHER:
                                ret = printOpUnary(buf, unsignInstr, iflg, "", false, false);
                                break;
                            default:
                                Global::specifyError("Invalid type encountered.\n", __FILE__, __LINE__);
                                throw Global::DeveloperError;
                        }
                        return ret;
                    }
                    case UOB::BINARY:
                    {
                        assertTypeErrorBinary(type());
                        int ret;
                        switch(type()){
                            case VarType::CHAR:
                                ret = printOpBinary(buf, unsignInstr, iflg, "i8 ", false, false);    
                                break;
                            case VarType::INT:
                                ret = printOpBinary(buf, signInstr, iflg, "i32 ", true, false);
                                break;
                            case VarType::LONG:
                                ret = printOpBinary(buf, unsignInstr, iflg, "i64 ", false, false);
                                break;
                            case VarType::BOOL:
                                ret = printOpBinary(buf, unsignInstr, iflg, "i1 ", false, false);
                                break;
                            case VarType::FLOAT:
                                if(__builtin_expect(std::strcmp(fltInstr, ERROR_INSTR) == 0,false)){
                                    Global::specifyError("Flt type used in int-only instruction.\n", __FILE__, __LINE__);    
                                    throw Global::InvalidInstrInvocation;
                                }
                                // right now, floats in Growl are 64-bit floats in LLVM.    
                                ret = printOpBinary(buf, fltInstr, fflg, dispFltType ? "float " : "", true, true);
                                break;
                            case VarType::OTHER:
                                ret = printOpBinary(buf, unsignInstr, iflg, "", false, false);
                                break;
                            default:
                                Global::specifyError("Invalid type encountered.\n", __FILE__, __LINE__);
                                throw Global::DeveloperError;
                        }
                        break;
                    }
                    default:
                        // big bad wolf, _Dmy node doesnt need type check anyway.
                        return 0;
                }
                // never happens
                return 0;
            }    
        /*
        enum class LLVMInstr:char {_Add, _Sub, _Mul, _Div, _Mod, _Shl, _Shr, 
            _And, _Or, _Xor, _Gr, _Ls, _Eq, _Dmy, _Asn, _Flp, _Neg, _Unid}; 
        enum class IntrOps:char {ADD, MINUS, NEG, MULT, DEREF, DIV, MOD, FLIP, DOT,
            GREATER, LESS, EQUAL, ADDRESS, AND, OR, XOR, ASSN, LSHIFT, RSHIFT, OTHER}; */
            LLVMInstr opToLLVM(IntrOps stype){
                switch(stype){
                    case IntrOps::ADD:
                        return LLVMInstr::_Add;
                    case IntrOps::MINUS:
                        return LLVMInstr::_Sub;
                    case IntrOps::MULT:
                        return LLVMInstr::_Mul;
                    case IntrOps::NEG:
                        return LLVMInstr::_Neg;
                    case IntrOps::FLIP:
                        return LLVMInstr::_Flp;
                    case IntrOps::DIV:
                        return LLVMInstr::_Div;
                    case IntrOps::MOD:
                        return LLVMInstr::_Mod;
                    case IntrOps::LSHIFT:
                        return LLVMInstr::_Shl;
                    case IntrOps::RSHIFT:
                        return LLVMInstr::_Shr;
                    case IntrOps::AND:
                        return LLVMInstr::_And;
                    case IntrOps::OR:
                        return LLVMInstr::_Or;
                    case IntrOps::XOR:
                        return LLVMInstr::_Xor;
                    case IntrOps::GREATER:
                        return LLVMInstr::_Gr;
                    case IntrOps::LESS:
                        return LLVMInstr::_Ls;
                    case IntrOps::EQUAL:
                        return LLVMInstr::_Eq;
                    case IntrOps::ASSN:
                        return LLVMInstr::_Asn;
                    default:
                        Global::specifyError("Operation not supported.", __FILE__, __LINE__);
                        throw Global::NotSupportedError;
                }
            }

        public:
            // just for default constr in vector.
            IInstr(){
                instr() = LLVMInstr::_Dmy;
                src1() = SSA::nullValue();
                src2() = SSA::nullValue();
                dest() = SSA::nullValue();
                type() = VarType::OTHER;
            }

            IInstr(VarType _Type, SSA _Src, SSA _Dest){
                src1() = _Src;
                src2() = SSA::nullValue();
                dest() = _Dest;
                instr() = LLVMInstr::_Dmy;
                type() = _Type;
            }

            void setInstr(IInstr& is){
                instr() = is.instr();
                if(is.instr() == LLVMInstr::_Br){
                    pred() = is.pred();
                    ifbr() = is.ifbr();
                    elsebr() = is.elsebr();
                } else if(is.instr() == LLVMInstr::_Cast){
                    iType() = is.iType();
                    oType() = is.oType();
                    cSrc() = is.cSrc();
                    cDest() = is.cDest();
                } else {
                    type() = is.type();
                    src1() = is.src1();
                    src2() = is.src2();
                    dest() = is.dest();
                }
            }

            // for branches.
            IInstr(SSA _Pred, Label _Ifbr, Label _Elsebr){
                instr() = LLVMInstr::_Br;
                pred() = _Pred;
                ifbr() = _Ifbr;
                elsebr() = _Elsebr;
            }

            void setBranch(SSA _Pred, Label _Ifbr, Label _Elsebr){
                instr() = LLVMInstr::_Br;
                pred() = _Pred;
                ifbr() = _Ifbr;
                elsebr() = _Elsebr;
            }

            // special for cast.
            IInstr(VarType _InType, VarType _OutType, SSA _Src, SSA _Dest){
                instr() = LLVMInstr::_Cast;
                cDest() = _Dest;
                iType() = _InType;
                oType() = _OutType;
                cSrc() = _Src;
            }

            void set(VarType _InType, VarType _OutType, SSA _Src, SSA _Dest){
                instr() = LLVMInstr::_Cast;
                cDest() = _Dest;
                iType() = _InType;
                oType() = _OutType;
                cSrc() = _Src;
            }

            inline bool isTwoInstrCast(){
                return iType() == VarType::LONG && oType() == VarType::INT;
            }

            // special for decl.
            IInstr(VarType _Type, SSA _Name){
                instr() = LLVMInstr::_Decl;
                dest() = _Name;
                type() = _Type;
                src1() = SSA::nullValue();
                src2() = SSA::nullValue();
            }

            void setDecl(VarType _Type, SSA _Name){
                instr() = LLVMInstr::_Decl;
                dest() = _Name;
                type() = _Type;
                src1() = SSA::nullValue();
                src2() = SSA::nullValue();
            }

            // for binary op.
            IInstr(IntrOps _Instr, VarType _Type, SSA _Src1, SSA _Src2, SSA _Dest){
                src1() = _Src1;
                src2() = _Src2;
                dest() = _Dest;
                instr() = opToLLVM(_Instr);
                type() = _Type;
            }

            void setBinOp(IntrOps _Instr, VarType _Type, SSA _Src1, SSA _Src2, SSA _Dest){
                src1() = _Src1;
                src2() = _Src2;
                dest() = _Dest;
                instr() = opToLLVM(_Instr);
                type() = _Type;
            }

            // for unary op.
            IInstr(IntrOps _Instr, VarType _Type, SSA _Src, SSA _Dest){
                src1() = _Src;
                src2() = SSA::nullValue();
                dest() = _Dest;
                instr() = opToLLVM(_Instr);
                type() = _Type;
            }

            void setUnOp(IntrOps _Instr, VarType _Type, SSA _Src, SSA _Dest){
                src1() = _Src;
                src2() = SSA::nullValue();
                dest() = _Dest;
                instr() = opToLLVM(_Instr);
                type() = _Type;
            }

            const char* typeToString(VarType v){
                switch(v){
                    case VarType::CHAR:
                        return "i8";
                    case VarType::INT:
                        return "i32";
                    case VarType::BOOL:
                        return "i1";
                    case VarType::LONG:
                        return "i64";
                    case VarType::FLOAT:
                        return "double";
                    default:
                        Global::specifyError("Invalid type.\n", __FILE__, __LINE__);
                        throw Global::DeveloperError;
                }
            }

            unsigned int output(char* buf){
                switch(instr()){
                    case LLVMInstr::_Br:
                        return brOutput(buf);
                    case LLVMInstr::_Add:
                        return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "add ", "add ", "fadd ", false);
                    case LLVMInstr::_Sub:
                        return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "sub ", "sub ", "fsub ", false);    
                    case LLVMInstr::_Mul:
                        return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "mul ", "mul ", "fmul ", false);
                    case LLVMInstr::_Div: 
                        return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "udiv ", "sdiv ", "fdiv ", false);
                    case LLVMInstr::_Mod:
                        return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "urem ", "srem ", "frem ", false);    
                    case LLVMInstr::_Shl:
                        return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "shl ", "shl ", ERROR_INSTR, false);    
                    case LLVMInstr::_Shr:
                        return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "lshr ", "ashr ", ERROR_INSTR, false);
                    case LLVMInstr::_And:
                        return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "and ", "and ", ERROR_INSTR, false);
                    case LLVMInstr::_Or:
                        return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "or ", "or ", ERROR_INSTR, false);
                    case LLVMInstr::_Xor:
                        return outputHelp(buf, EMPTY_FLAG, EMPTY_FLAG, "xor ", "xor ", ERROR_INSTR, false);
                    case LLVMInstr::_Gr:
                        return outputHelp(buf, OpUtils::isUnsigned(type()) ? "ugt " : "sgt ", 
                                "ugt ", "icmp ", "icmp ", "fcmp ", true);
                    case LLVMInstr::_Ls:
                        return outputHelp(buf, OpUtils::isUnsigned(type()) ? "ult " : "slt ", 
                                "ult ", "icmp ", "icmp ", "fcmp ", true);
                    case LLVMInstr::_Eq:
                        return outputHelp(buf, OpUtils::isUnsigned(type()) ? "eq " : "eq ", 
                                "ueq ", "icmp ", "icmp ", "fcmp ", true);
                    case LLVMInstr::_Dmy:
                        return outputHelp(buf, "", "", "", "", "", false);
                    // write out a construction.
                    case LLVMInstr::_Asn:
                    {
                        return assnOutput(buf);
                    }
                    case LLVMInstr::_Cast:
                        return castFmt(buf); 
                    case LLVMInstr::_Decl:
                    {
                        return std::snprintf(buf, INSTR_BUF_SIZE, "%%s%u = alloca %s",
                                        dest().extractLbl(), typeToString(type()));
                    }
                    case LLVMInstr::_Flp:
                    {
                        unsigned long long neg1 = OpUtils::genAllOne(type());
                        if(neg1 == 0){
                            return IInstr(IntrOps::XOR, type(), src1(), 
                                            SSA::genSInt(-1), dest()).output(buf);
                        } else {
                            return IInstr(IntrOps::XOR, type(), 
                                            src1(), neg1, dest()).output(buf);
                        }
                    }
                    case LLVMInstr::_Neg:
                    {
                        if(OpUtils::isFloat(type())){
                            return IInstr(IntrOps::MINUS, type(), SSA::genFlt(0), 
                                            src1(), dest()).output(buf);
                        } else {
                            return IInstr(IntrOps::MINUS, type(), SSA::genSInt(0), 
                                            src1(), dest()).output(buf);
                        }
                    }
                }
                return 0;
            }
            SSA* getDest(){
                return &dest(); 
            }
    };

    Label getFromAST(unsigned int AST_Extract);
    void insertASTLbl(unsigned int AST_Extr, Label lbl);

    class IRProg {
        private:
            // labels are 1-indexed. No collisions.
            Utils::SmallVector<unsigned, 20> map;
            Utils::SmallVector<CodeGen::IInstr, 1> list;
        public:
            IRProg(){
            }

            ~IRProg(){
            }

            IInstr* getInstr(Label& labl){
                return list.begin() + map[labl.extract()];
            }

            unsigned int size(){
                return list.size();
            }

            inline void allocate(int howMany){
                list.allocate(howMany);
            }

            inline void addInstr(IInstr& instr){
                list.allocate(1);
                list.back()->setInstr(instr);
            }

            /** Only safe when not adding */
            inline IInstr* getInstr(unsigned idx){
                return list.begin() + idx;
            }

            void addInstr(Label& lbl, IInstr& instr){
                addInstr(instr);
                map.ref(lbl.extract()) = list.size() - 1;
            }

            void associate(Label lbl, unsigned idx){
                map.ref(lbl.extract()) = map.ref(idx);
            }

            void write(std::ostream& out){
                char writeBuf[INSTR_BUF_SIZE+1];
                for(auto iter = list.begin(); iter != list.end(); ++iter){
                    iter->output(writeBuf);
                    out << writeBuf;
                    out << '\n';
                }
                out.flush();
            } 
    };

    unsigned int genIR(IRProg& program);
    void genASM(Utils::Vector<IInstr>& buf);

    IRProg& getIRProg();
}

#endif
