
#include "Lex.h"
#include "Syntax.h"
#include "Error.h"

using namespace Lex;
using Syntax::OpType;
using Syntax::Assoc;

OpType Syntax::opType(SubType type) {
    switch(type) {
        case AMP:
        case ASTK:
        case MINUS:
            return OpType::AMBIG_TYPE;
        case NEG:
            return OpType::UNARY;
        case ASSN:
        case CARET:
        case DIV:
        case DOT:
        case EQUAL:
        case GREATER:
        case LESS:
        case MOD:
        case OR:
        case PLUS:
        case SHIFT:
            return OpType::BINARY;
        default:
            throw Global::InvalidOperator;
    }
}

Syntax::Assoc Syntax::associate(SubType type, OpType opType) {
    switch(type) {
        case NEG:
        case GREATER:
        case LESS:
            return Assoc::NONE;
        case AMP:
        case ASTK:
        case MINUS:
            switch(opType) {
                case UNARY:
                    return NONE;
                case AMBIG_TYPE:
                    return AMBIG;
                case BINARY:
                    return LEFT;
                default:
                    throw Global::DeveloperError;
            }
        case ASSN:
            return RIGHT;
        case CARET:
        case DIV:
        case DOT:
        case EQUAL:
        case MOD:
        case OR:
        case PLUS:
        case SHIFT:
            return LEFT;
        default:
            throw Global::InvalidOperator;
    }
}

// error if called with ambigious opType. must be resolved.
int Syntax::precedence(SubType type, OpType opType) {
    using namespace Lex;
    switch(type) {
        case DOT:
            return 11;
        case NEG:
            return 10;
        case ASTK:
            switch(opType) {
                case UNARY:
                    return 10;
                case BINARY:
                    return 9;
                case AMBIG_TYPE:
                default:
                    throw Global::DeveloperError;
            }
        case DIV:
        case MOD:
            return 9;
        case MINUS:
            switch(opType) {
                case UNARY:
                    return 10;
                case BINARY:
                    return 8;
                case AMBIG_TYPE:
                default:
                    throw Global::DeveloperError;
            }
        case PLUS:
            return 8;
        case SHIFT:
            return 7;
        case GREATER:
        case LESS:
            return 6;
        case EQUAL:
            return 5;
        case AMP:
            switch(opType) {
                case UNARY:
                    return 10;
                case BINARY:
                    return 4;
                case AMBIG_TYPE:
                default:
                    throw Global::DeveloperError;
            }
        case CARET:
            return 3;
        case OR:
            return 2;
        case ASSN:
            return 1;
        default:
            throw Global::InvalidOperator;
    }
}