
#include "Parse.h"
#include "Error.h"
#include "AST.hpp"

/*
Implements a polymorphic iterator.
Gets around the annoying features for C++ covariance/ avoids heap alloc.

This is very useful for traversing nodes with variable length
features.

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

ArgIterator::ArgIterator(SupportedType mType, AST* mHandle){
    type = mType;
    handle = mHandle;
	pos = 0;
}

bool ArgIterator::done(){
    switch(type){
        case SupportedType::_Op:
            return pos == (static_cast<Op*>(handle))->arity();
		case SupportedType::_Seq:
			return pos == (static_cast<Sequence*>(handle))->size();
		case SupportedType::_Br:
			return static_cast<Branch*>(handle)->next == nullptr;
        case SupportedType::_Lit:
        case SupportedType::_Var:
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
		case SupportedType::_Seq:
            ++pos;
            break;
		case SupportedType::_Ctl:
            Global::specifyError("Global scope is being traversed. ArgIterator.");
            throw Global::DeveloperError;
		case SupportedType::_Br:
			handle = static_cast<Branch*>(handle)->next;
			break;
        case SupportedType::_Lit:
        case SupportedType::_Var:
		case SupportedType::_Decl:
		case SupportedType::_Lp:
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
                    return _op->inputs.twoArgs[pos];
                default:
                    // a function for sure.
                    return _op->inputs.args[pos];
            }
            break;
		}
		case SupportedType::_Seq:
		{
			Sequence* _seq = static_cast<Sequence*>(handle);
			return _seq->at(pos);
		}
		case SupportedType::_Br:
        	return handle;
		case SupportedType::_Lit:
        case SupportedType::_Var:
		case SupportedType::_Decl:
		case SupportedType::_Lp:
		case SupportedType::_Ctl:
		default:
            Global::specifyError("Should never happen. PolyIter:get.");
            throw Global::DeveloperError;
    }
}
