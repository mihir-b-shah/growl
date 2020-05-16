
package parser.parsetypes.ast;

import lexer.lextypes.Operator;
import parser.parsetypes.enums.OpType;

/**
 * Base class for operator.
 * 
 * @author mihir
 */
public abstract class Op<T extends AST> extends Expression {
        
    public Operator operator;
    public abstract OpType type();
}
