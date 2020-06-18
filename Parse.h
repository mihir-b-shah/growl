
#ifndef PARSE_H
#define PARSE_H

#include "AST.h"
#include "Lex.h"

namespace Parse {
    Expr* parseExpr(Lex::Token* begin, Lex::Token* end);
}

#endif