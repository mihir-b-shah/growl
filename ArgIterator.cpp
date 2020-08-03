
#include "Parse.h"
#include "Error.h"
#include "AST.hpp"

/*
Implements a polymorphic iterator.
Gets around the annoying features for C++ covariance/ avoids heap alloc.
*/

using namespace Parse;

/*
enum class SupportedType:char {_Expr, _Op, _Lit, _Var, 
							   _Decl, _Lp, _Br, _Ctl, _Seq};
    
class ArgIterator {
    SupportedType type;
    Expr* handle;

    union {
        int pos;
        void* obj;
    } aux;

	_Op: traverse the args of a function
	_Lit: nothing
	_Var: nothing
	_Decl: nothing
	_Lp: nothing
	_Br: nothing
	_Ctl: nothing
	_Seq: traverse the sequence
*/

ArgIterator::ArgIterator(SupportedType mType, AST* mHandle, void* pObj){
    type = mType;
    handle = mHandle;
    aux.obj = pObj;
}

ArgIterator::ArgIterator(SupportedType mType, AST* mHandle, int mPos){
    type = mType;
    handle = mHandle;
    aux.pos = mPos;
}

bool ArgIterator::done(){
    switch(type){
        case SupportedType::_Op:
            return aux.pos == (static_cast<Op*>(handle))->arity();
		case SupportedType::_Seq:
			return aux.pos == (static_cast<Sequence*>(handle))->size();
        case SupportedType::_Lit:
        case SupportedType::_Var:
		case SupportedType::_Br:
		case SupportedType::_Decl:
		case SupportedType::_Lp:
			return true;
		case SupportedType::_Ctl:
            Global::specifyError("Global scope is being traversed. ArgIterator.");
            throw Global::DeveloperError;
        default:
            Global::specifyError("Should never happen. PolyIter:done.");
            throw Global::DeveloperError;
    }
}

void ArgIterator::next(){
    switch(type){
        case SupportedType::_Op:
            ++aux.pos;
            break;
        case SupportedType::_Lit:
        case SupportedType::_Var:
            Global::specifyError("Should never happen. PolyIter:next.");
            throw Global::DeveloperError;
		case SupportedType::_Br:
			return;
		case SupportedType::_Ctl:
            Global::specifyError("Global scope is being traversed. ArgIterator.");
            throw Global::DeveloperError;
		case SupportedType::_Decl:
			return;
		case SupportedType::_Seq:
			break;
		case SupportedType::_Lp:
			return;
        default:
            Global::specifyError("Should never happen. PolyIter:next.");
            throw Global::DeveloperError;
    }
}

AST* ArgIterator::get(){
    switch(type){
        case SupportedType::_Op:
        {
            Op* _op = static_cast<Op*>(handle);
            switch(_op->arity()){
                case 1:
                    // unary operator or unary function.
                    return _op->inputs.arg;
                case 2:
                    // binary operator or binary function.
                    return _op->inputs.twoArgs[aux.pos];
                default:
                    // a function for sure.
                    return _op->inputs.args[aux.pos];
            }
            break;
        }
        case SupportedType::_Lit:
        case SupportedType::_Var:
        default:
            Global::specifyError("Should never happen. PolyIter:get.");
            throw Global::DeveloperError;
    }
}
