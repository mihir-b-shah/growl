
package parser.parsetypes.ast;

import debug.Debug;
import lexer.Lexeme;

/**
 * @author mihir
 */
public abstract class Expression extends AST {
    public static Expression genLitVal(Lexeme lexeme) {
        switch(lexeme.type) {
            case LITERAL:
                return new Literal(lexeme);
            case ID:
                return new Variable(lexeme);
            default:
                return null;
        }
    }
    
    public abstract Number evalConstExpr();
    
    // very inefficient but its fine in a debugging utility
    @Override
    public String toString() {
        return display(Debug.CONSOLE_WIDTH/2, Debug.CONSOLE_WIDTH/2);
    }
    
    public abstract String display(int pos, int width);
}
