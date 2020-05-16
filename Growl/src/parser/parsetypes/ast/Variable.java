
package parser.parsetypes.ast;

import lexer.*;
import debug.Debug;
import lexer.lextypes.LiteralType;
import lexer.lextypes.Operator;
import lexer.token.StringToken;

/**
 * @author mihir
 */
public class Variable extends Expression {
    public LiteralType type;
    public String id;

    public Variable() {
    }

    public Variable(Lexeme lexeme) {
        assert(lexeme.type == LexType.ID);
        type = ((LiteralType) lexeme.subType);
        id = ((StringToken) lexeme.token).token;
    }

    @Override
    public Number evalConstExpr() {
        return Operator.UNSUPPORTED;
    }

    @Override
    public String display(int pos, int width) {
        return Debug.UNSUPPORTED;
    }
}