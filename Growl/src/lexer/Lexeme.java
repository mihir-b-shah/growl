
package lexer;

import lexer.token.Token;

/**
 * Encapsulates tokens from program.
 * 
 * @author mihir
 */
public class Lexeme {

    public final LexType type;
    public Object subType; // either class or enum
    public final Token token;
    final int start;
    final int end;

    Lexeme(LexType type, Token token, int start, int end) {
        this.type = type;
        this.token = token;
        this.start = start;
        this.end = end;
    }

    @Override
    public String toString() {
        return String.format("%s %s, %s", type.toString(), subType != null
                ? subType.toString() : "Id", token.toString());
    }
}
