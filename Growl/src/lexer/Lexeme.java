
package lexer;

import lexer.token.Token;

/**
 * Encapsulates tokens from program.
 * 
 * @author mihir
 */
public class Lexeme {

    public LexType type;
    public Object subType; // either class or enum
    public Token token;
    int start;
    int end;

    Lexeme(LexType type, Token token, int start, int end) {
        this.type = type;
        this.token = token;
        this.start = start;
        this.end = end;
    }

    @Override
    public String toString() {
        return String.format("%s-%s, %s", type.toString(), subType != null
                ? subType.toString() : "Id", token);
    }
}
