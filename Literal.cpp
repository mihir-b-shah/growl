
#include "AST.h"

using namespace Parse;

Literal::Literal() {
}

Literal::~Literal(){
}

bool Literal::isInt(){
    return type == INT;
}

bool Literal::isFloat(){
    return type == FLOAT;
}

long long Literal::getInt(){
    return value.intVal;
}
    
long double Literal::getFlt(){
    return value.fltVal;
}
