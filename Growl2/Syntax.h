
#include "Lex.h"

namespace Syntax {
    enum OpType {
        AMBIG_TYPE, UNARY, BINARY
    };

    // garbage in garbage out. if you throw ambigious optype get ambig associativity.
    // in the case where its ambigious in first place.
    enum Assoc {
        AMBIG, NONE, RIGHT, LEFT // ambigious between two...
    };

    OpType opType(Lex::SubType type);
    Assoc associate(Lex::SubType type, OpType optype);
    int precedence(Lex::SubType type, Syntax::OpType opType);
}