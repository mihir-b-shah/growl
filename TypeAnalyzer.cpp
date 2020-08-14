
#include "AST.hpp"
#include "CodeGen.hpp"
#include "Error.h"
#include <utility>

using namespace Parse;

/** Some definitions for Cast */
Cast::Cast(Expr* _in, VarType _out){
    in = _in;
    out = _out;
}

Cast::~Cast(){
}

/** Indices: INT=0, LONG=1, CHAR=2, FLOAT=3, BOOL=4 */
static const VarType castLookup[5][5] = {
    {VarType::INT, VarType::LONG, VarType::INT, VarType::FLOAT, VarType::INT},
    {VarType::LONG, VarType::LONG, VarType::LONG, VarType::FLOAT, VarType::LONG},
    {VarType::INT, VarType::LONG, VarType::CHAR, VarType::FLOAT, VarType::CHAR},
    {VarType::FLOAT, VarType::FLOAT, VarType::FLOAT, VarType::FLOAT, VarType::FLOAT},
    {VarType::INT, VarType::LONG, VarType::CHAR, VarType::FLOAT, VarType::BOOL}
};

static std::pair<VarType,unsigned> getNewType(VarType one, VarType two){
    VarType comb = castLookup[static_cast<int>(one)][static_cast<int>(two)]; 
    return {comb, comb == one}; // gives 1 if need to change #2, 0 if need to fix #1.
}

// Type analysis stuff.
// RUN TOYPROG.GRR, see the lack of casting in assignment operation.
VarType Op::castType(){
    switch(this->arity()){
        case 1:
            // unary operators never need casting.
            break;
        case 2:
        {
            VarType arg1 = this->getBinaryArg1()->castType();
            VarType arg2 = this->getBinaryArg2()->castType();
            
            if(arg1 != arg2) {                   
                if(this->getIntrinsicOp() == IntrOps::ASSN){
                    if(__builtin_expect(this->getBinaryArg1()->exprID() != ExprId::_VAR,false)){
                        Global::specifyError("Assignment to non-variable.\n", __FILE__, __LINE__);
                        throw Global::InvalidExpression;
                    }
                    /* Implements the special cast-less behavior for literals */
                    Literal* lit;
                    if((lit = dynamic_cast<Literal*>(this->getBinaryArg2())) 
                                    && lit->isFloat()){
                        lit->convType(false);
                    } else if(lit == nullptr){
                        Cast* assnCast = new Cast(this->getBinaryArg2(), arg1);
                        this->setBinaryArg2(assnCast);
                    }
                    _in = arg1;
                    return arg1;
                }
                auto ret = getNewType(arg1, arg2);
                Cast* cast;
                Literal* lit;
                if(ret.second == 0){
                    // analysis relies on fact FLOAT is a supertype: anything can promote to it.
                    if(lit = dynamic_cast<Literal*>(this->getBinaryArg1())){
                        lit->convType(true);
                    } else if(dynamic_cast<Literal*>(this->getBinaryArg2()) == NULL
                                || (arg1==VarType::FLOAT xor arg2==VarType::FLOAT)) {
                        // the other type is not a literal of the same class INT/FLOAT
                        std::cout << (arg1==VarType::FLOAT xor arg2==VarType::FLOAT) << '\n';
                        cast = new Cast(this->getBinaryArg1(), ret.first);
                        this->setBinaryArg1(cast);
                    } else {
                        // since theres only one type of float.
                        _in = arg2;
                        return arg2;
                    }
                } else {
                    if(lit = dynamic_cast<Literal*>(this->getBinaryArg2())){
                        lit->convType(true);
                    } else if(dynamic_cast<Literal*>(this->getBinaryArg1()) == NULL 
                                || (arg1==VarType::FLOAT xor arg2==VarType::FLOAT)) {
                        cast = new Cast(this->getBinaryArg2(), ret.first);
                        this->setBinaryArg2(cast);
                    } else {
                       _in = arg1;
                       return arg1;
                    }
                }
                _in = ret.first;
                return ret.first;
            }
                
            _in = arg1;
            return arg1;
        }
        default:
            Global::specifyError("Functions (n-ary) op not supported.\n",
                            __FILE__, __LINE__);
            throw Global::NotSupportedError;
    }
    _in = VarType::OTHER;
    return VarType::OTHER;
}

// Maximum width. Literals should never be the limiter.
VarType Literal::castType(){
    _in = this->isInt() ? VarType::LONG : VarType::FLOAT;
    return _in;
}

VarType Variable::castType(){
    _in = this->type;
    return this->type;
}

// guaranteed that the object is of the opposite type.
void Literal::convType(bool fltType){
    if(fltType){
        this->setFlt(static_cast<double>(this->getInt()));    
    } else {
        this->setInt(static_cast<unsigned long long>(this->getFlt())); 
    }
}

void Control::fixTypes(){
    return this->getSeq()->fixTypes();
}

void Expr::fixTypes(){
    this->castType();
    this->print(80, std::cout);
}

void Sequence::fixTypes(){
    for(auto iter = this->iterator(); !iter.done(); iter.next()){
        iter.get()->fixTypes();
    }
}

// T should extend Control*
template<typename T>
static inline void handlePredicate(T* str){
    VarType outType = str->getPred()->castType();
    if(outType != VarType::BOOL){
        Cast* cast = new Cast(str->getPred(), VarType::BOOL);
        str->setPred(cast);
    }
    str->getPred()->print(80, std::cout);
}

void Branch::fixTypes(){
    for(auto iter = this->iterator(); !iter.done(); iter.next()){
        Branch* br = static_cast<Branch*>(iter.get());
        handlePredicate<Branch>(br);
        br->getSeq()->fixTypes();
    }
}

void Loop::fixTypes(){
    handlePredicate<Loop>(this);
    this->getSeq()->fixTypes();
}

