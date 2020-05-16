
package parser.parsetypes.ast;

import lexer.LexType;
import lexer.Lexeme;
import lexer.lextypes.LiteralType;
import lexer.token.Token;

/**
 * @author mihir
 */
public class Literal extends Expression {
    public LiteralType type;
    public Token value;

    public Literal() {
    }

    public Literal(Lexeme lexeme) {
        assert(lexeme.type == LexType.LITERAL);
        type = ((LiteralType) lexeme.subType);
        value = lexeme.token;
    }

    @Override
    public Number evalConstExpr() {
        return value.getValue();
    }

    @Override
    public String display(int pos, int width) {
       return value.toString();
    }
}
