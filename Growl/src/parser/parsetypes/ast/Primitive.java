
package parser.parsetypes.ast;

import debug.Debug;
import lexer.lextypes.DataType;
import lexer.lextypes.Operator;

/**
 *
 * @author mihir
 */
public class Primitive extends Value {
        
    public DataType.BaseType type;
    public Expression supplier;

    @Override
    public Number evalConstExpr() {
        return Operator.UNSUPPORTED;
    }

    @Override
    public String display(int pos, int width) {
        return Debug.UNSUPPORTED;
    }
}
