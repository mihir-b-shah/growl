
package parser.parsetypes.ast;

import lexer.lextypes.DataType;

/**
 *
 * @author mihir
 */
public class Bool extends Primitive {
    // just a marker class for the loop/branch conditions
    public Bool() {
        type = DataType.BaseType.BOOL;
    }
}
