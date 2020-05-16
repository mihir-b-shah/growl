
package parser.parsetypes.ast;

import debug.Debug;
import lexer.lextypes.Operator;

/**
 *
 * @author mihir
 */
public class Struct extends Value {

    Expression supplier;
    StructDef definition;

    @Override
    public Number evalConstExpr() {
        return Operator.UNSUPPORTED;
    }

    @Override
    public String display(int pos, int width) {
        return Debug.UNSUPPORTED;
    }
}
