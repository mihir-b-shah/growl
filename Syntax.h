
#ifndef SYNTAX_H
#define SYNTAX_H

#include "Lex.h"

using Lex::SubType;

namespace Syntax {
    enum class OpType:char {
        AMBIG_TYPE, UNARY, BINARY
    };

    // garbage in garbage out. if you throw ambigious optype get ambig associativity.
    // in the case where its ambigious in first place.
    enum class Assoc:char {
        AMBIG, NONE, RIGHT, LEFT // ambigious between two...
    };

    int optypeInt(OpType optype);
    OpType opType(SubType type);
    Assoc associate(SubType type, OpType optype);
    int precedence(SubType type, OpType opType);
}

#endif
