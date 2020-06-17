
#include "Lex.h"
#include "Syntax.h"
#include "Error.h"

using namespace Lex;
using Syntax::OpType;
using Syntax::Assoc;

OpType Syntax::opType(SubType type) {
    switch(type) {
        case SubType::AMP:
        case SubType::ASTK:
        case SubType::MINUS:
            return OpType::AMBIG_TYPE;
        case SubType::NEG:
            return OpType::UNARY;
        case SubType::ASSN:
        case SubType::CARET:
        case SubType::DIV:
        case SubType::DOT:
        case SubType::EQUAL:
        case SubType::GREATER:
        case SubType::LESS:
        case SubType::MOD:
        case SubType::OR:
        case SubType::PLUS:
        case SubType::SHIFT:
            return OpType::BINARY;
        default:
            throw Global::InvalidOperator;
    }
}

Syntax::Assoc Syntax::associate(SubType type, OpType opType) {
    switch(type) {
        case SubType::NEG:
        case SubType::GREATER:
        case SubType::LESS:
            return Assoc::NONE;
        case SubType::AMP:
        case SubType::ASTK:
        case SubType::MINUS:
            switch(opType) {
                case OpType::UNARY:
                    return Assoc::NONE;
                case OpType::AMBIG_TYPE:
                    return Assoc::AMBIG;
                case OpType::BINARY:
                    return Assoc::LEFT;
                default:
                    throw Global::DeveloperError;
            }
        case SubType::ASSN:
            return Assoc::RIGHT;
        case SubType::CARET:
        case SubType::DIV:
        case SubType::DOT:
        case SubType::EQUAL:
        case SubType::MOD:
        case SubType::OR:
        case SubType::PLUS:
        case SubType::SHIFT:
            return Assoc::LEFT;
        default:
            throw Global::InvalidOperator;
    }
}

// error if called with ambigious opType. must be resolved.
int Syntax::precedence(SubType type, OpType opType) {
    using namespace Lex;
    switch(type) {
        case SubType::DOT:
            return 11;
        case SubType::NEG:
            return 10;
        case SubType::ASTK:
            switch(opType) {
                case OpType::UNARY:
                    return 10;
                case OpType::BINARY:
                    return 9;
                case OpType::AMBIG_TYPE:
                default:
                    throw Global::DeveloperError;
            }
        case SubType::DIV:
        case SubType::MOD:
            return 9;
        case SubType::MINUS:
            switch(opType) {
                case OpType::UNARY:
                    return 10;
                case OpType::BINARY:
                    return 8;
                case OpType::AMBIG_TYPE:
                default:
                    throw Global::DeveloperError;
            }
        case SubType::PLUS:
            return 8;
        case SubType::SHIFT:
            return 7;
        case SubType::GREATER:
        case SubType::LESS:
            return 6;
        case SubType::EQUAL:
            return 5;
        case SubType::AMP:
            switch(opType) {
                case OpType::UNARY:
                    return 10;
                case OpType::BINARY:
                    return 4;
                case OpType::AMBIG_TYPE:
                default:
                    throw Global::DeveloperError;
            }
        case SubType::CARET:
            return 3;
        case SubType::OR:
            return 2;
        case SubType::ASSN:
            return 1;
        default:
            throw Global::InvalidOperator;
    }
}