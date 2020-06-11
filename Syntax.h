
#ifndef SYNTAX_H
#define SYNTAX_H

#include "Lex.h"

using Lex::SubType;

namespace Syntax {
    enum OpType {
        AMBIG_TYPE, UNARY, BINARY
    };

    // garbage in garbage out. if you throw ambigious optype get ambig associativity.
    // in the case where its ambigious in first place.
    enum Assoc {
        AMBIG, NONE, RIGHT, LEFT // ambigious between two...
    };

    OpType opType(SubType type);
    Assoc associate(SubType type, OpType optype);
    int precedence(SubType type, OpType opType);
}

#endif