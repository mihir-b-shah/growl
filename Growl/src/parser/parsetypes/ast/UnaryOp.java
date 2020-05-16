/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package parser.parsetypes.ast;

import lexer.lextypes.Operator;
import debug.Debug;
import lexer.Lexeme;
import parser.parsetypes.enums.OpType;

/**
 *
 * @author mihir
 * @param <T> type
 */
// T allows customizing to a constexpr
public class UnaryOp<T extends AST> extends Op<T> {

    public T op;

    @Override
    public OpType type() {
        return OpType.UNARY;
    }

    public UnaryOp() {
    }
    
    public UnaryOp(Lexeme lexeme) {
        Operator ope = (Operator) lexeme.subType;
        operator = ope;
    }
    
    @Override
    public Number evalConstExpr() {
        if(op instanceof Expression) {
            Number res = operator.evaluateUnary(((Expression) op).evalConstExpr());
            return res;
        }
        return Operator.UNSUPPORTED;
    }

    @Override
    public String display(int pos, int width) {
        return Debug.UNSUPPORTED;
    }
}
