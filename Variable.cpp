
#include "AST.hpp"
#include "Lex.h"
#include "Error.h"

using namespace Parse;

void Variable::set(const char* _name, char _len, Lex::SubType _type, char _ptrLvl){
	name = _name;
    len = _len;
    switch(_type){
        case Lex::SubType::INT:
            type = VarType::INT;
            break;
        case Lex::SubType::LONG:
            type = VarType::LONG;
            break;
        case Lex::SubType::CHAR:
            type = VarType::CHAR;
            break;
        case Lex::SubType::FLOAT:
            type = VarType::FLOAT;
            break;
        case Lex::SubType::BOOL:
            type = VarType::BOOL;
            break;
        case Lex::SubType::VOID:
            type = VarType::VOID;
            break;
        default:
            Global::specifyError("Invalid variable type.");
            throw Global::DeveloperError; // should never occur.
    }
    ptrLvl = _ptrLvl;
    _unsigned = false; 
	// RIGHT NOW, IGNORE UNSIGNED VALUES. WILL INCORPORATE THEM LATER.
}

Variable::Variable(const char* _name, char _len, Lex::SubType _type, char _ptrLvl){
	set(_name, _len, _type, _ptrLvl);
}

Variable::~Variable(){
}

static inline int min(int a, int b){
    return a<b?a:b;
}

int Variable::printRoot(char* buf) const {
    return min(std::snprintf(buf,1+min(3,len),"%s",name),3);
}

Parse::ArgIterator Variable::iterator() {
    return ArgIterator(SupportedType::_Var, this);
}
