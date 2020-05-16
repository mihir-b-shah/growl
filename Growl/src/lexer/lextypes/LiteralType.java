
package lexer.lextypes;

import java.util.regex.Pattern;
import lexer.Lexer;
import lexer.token.*;

/**
 * @author mihir
 */
public enum LiteralType {
    BOOL_TRUE, BOOL_FALSE, INT, FLOAT;

    public static final Pattern LITERAL_REGEX = Pattern.compile("(true)|(false)"
            + "|('(\\\\)?.')|(((\\-)?(0b[01]+)|((\\-)?0x[0-9a-f]+)|((\\-)?[0-9]+))[SL]?)|((\\-)?\\d+\\.\\d+)");

    public static LiteralType createLiteral(Token s) {
        // why isnt getClass() enumerable... grrrr...
        // too many if statements nooooo
        if(s.getClass() == IntToken.class) {
            IntToken tok = (IntToken) s;
            switch(tok.size) {
                case Limits.CHAR_SIZE:
                    if(tok.bool) {
                        if(tok.token == 0) {
                            return LiteralType.BOOL_TRUE;
                        } else {
                            return LiteralType.BOOL_FALSE;
                        }
                    } else {
                        // char is synactic sugar over 8-bit integer
                        // only type determines what gets printed.
                        return LiteralType.INT;
                    }
                case Limits.INT_SIZE:
                case Limits.LONG_SIZE:
                    return LiteralType.INT;
                default:
                    throw new Lexer.LexError();
            }
        } else if(s.getClass() == FloatToken.class) {
            return LiteralType.FLOAT;
        } else {
            // later, the string will rise...
            throw new Lexer.LexError();
        }
    }
}