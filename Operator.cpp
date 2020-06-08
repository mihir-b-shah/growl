
#include "Syntax.h"
#include "Error.h"

Syntax::OpType Syntax::opType(Lex::SubType type) {
    using namespace Lex;
    switch(type) {
        case SubType::AMP:
        case SubType::ASTK:
        case SubType::MINUS:
            return Syntax::OpType::AMBIG_TYPE;
        case SubType::NEG:
            return Syntax::OpType::UNARY;
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
            return Syntax::OpType::BINARY;
        default:
            throw Global::InvalidOperator;
    }
}

Syntax::Assoc Syntax::associate(Lex::SubType type, Syntax::OpType opType) {
    using namespace Lex;
    switch(type) {
        case SubType::NEG:
        case SubType::GREATER:
        case SubType::LESS:
            return Syntax::Assoc::NONE;
        case SubType::AMP:
        case SubType::ASTK:
        case SubType::MINUS:
            switch(opType) {
                case Syntax::OpType::UNARY:
                    return Syntax::Assoc::NONE;
                case Syntax::OpType::AMBIG_TYPE:
                    return Syntax::Assoc::AMBIG;
                case Syntax::OpType::BINARY:
                    return Syntax::Assoc::LEFT;
                default:
                    throw Global::DeveloperError;
            }
        case SubType::ASSN:
            return Syntax::Assoc::RIGHT;
        case SubType::CARET:
        case SubType::DIV:
        case SubType::DOT:
        case SubType::EQUAL:
        case SubType::MOD:
        case SubType::OR:
        case SubType::PLUS:
        case SubType::SHIFT:
            return Syntax::Assoc::LEFT;
        default:
            throw Global::InvalidOperator;
    }
}

// error if called with ambigious opType. must be resolved.
int Syntax::precedence(Lex::SubType type, Syntax::OpType opType) {
    using namespace Lex;
    switch(type) {
        case SubType::DOT:
            return 11;
        case SubType::NEG:
            return 10;
        case SubType::ASTK:
            switch(opType) {
                case Syntax::OpType::UNARY:
                    return 10;
                case Syntax::OpType::BINARY:
                    return 9;
                case Syntax::OpType::AMBIG_TYPE:
                default:
                    throw Global::DeveloperError;
            }
        case SubType::DIV:
        case SubType::MOD:
            return 9;
        case SubType::MINUS:
            switch(opType) {
                case Syntax::OpType::UNARY:
                    return 10;
                case Syntax::OpType::BINARY:
                    return 8;
                case Syntax::OpType::AMBIG_TYPE:
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
                case Syntax::OpType::UNARY:
                    return 10;
                case Syntax::OpType::BINARY:
                    return 4;
                case Syntax::OpType::AMBIG_TYPE:
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