
#include "Parse.h"
#include "Error.h"

/*
Implements a polymorphic iterator.
Gets around the annoying features for C++.
*/

using namespace Parse;

/*
enum class SupportedType {_Expr, _Op, _Lit};
	
class ArgIterator {
	SupportedType type;
	Expr* handle;
	AuxState state;
	
	union {
		int pos;
		void* obj;
	} aux;
*/

ArgIterator::ArgIterator(SupportedType mType, Expr* mHandle, void* pObj){
	type = mType;
	handle = mHandle;
	aux.obj = pObj;
}

ArgIterator::ArgIterator(SupportedType mType, Expr* mHandle, int mPos){
	type = mType;
	handle = mHandle;
	aux.pos = mPos;
}

bool ArgIterator::done(){
	switch(mType){
		case SupportedType::_Op:
			return aux.pos == (static_cast<Op*>(handle))->arity();
		case SupportedType::_Lit:
			return true;
		default:
			Global::specifyError("Should never happen. PolyIter:done.");
			throw Global::DeveloperError;
	}
}

void ArgIterator::next(){
	switch(mType){
		case SupportedType::_Op:
			++aux.pos;
			break;
		case SupportedType::_Lit:
		default:
			Global::specifyError("Should never happen. PolyIter:next.");
			throw Global::DeveloperError;
	}
}

Expr* ArgIterator::get(){
	switch(mType){
		case SupportedType::_Op:
		{
			Op* _op = static_cast<Op*>(handle);
			switch(handle->arity()){
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
		case SupportedType::_Lit:
		default:
			Global::specifyError("Should never happen. PolyIter:get.");
			throw Global::DeveloperError;
	}
}