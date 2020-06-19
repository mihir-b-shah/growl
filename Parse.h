
#ifndef PARSE_H
#define PARSE_H

#include "AST.hpp"
#include "Lex.h"

namespace Parse {
    Parse::Expr* parseExpr(Lex::Token* begin, Lex::Token* end);
}

#endif