
package parser.parsetypes.ast;

import lexer.lextypes.Operator;
import debug.Debug;
import lexer.Lexeme;
import parser.parsetypes.enums.OpType;

/**
 * Binary operator
 * 
 * @author mihir
 * @param <T> tag
 */
public class BinaryOp<T extends AST> extends Op<T> {

    public T op1;
    public T op2;

    @Override
    public OpType type() {
        return OpType.BINARY;
    }

    public BinaryOp() {
        
    }
    
    public BinaryOp(Lexeme lexeme) {
        Operator ope = (Operator) lexeme.subType;
        operator = ope;
    }

    @Override
    public Number evalConstExpr() {
        if(op1 instanceof Expression && op2 instanceof Expression) {
            Number res = operator.evaluateBinary(((Expression) op1).evalConstExpr(), 
                    ((Expression) op2).evalConstExpr());
            return res;
        }
        return Operator.UNSUPPORTED;
    }

    @Override
    public String display(int pos, int width) {
        return Debug.UNSUPPORTED;
    }
}
