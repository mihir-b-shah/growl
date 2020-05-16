
package lexer.token;

/**
 * Superclass for tokens. Easier way to represent and operate
 * other than strings.
 * 
 * @author mihir
 */
public abstract class Token {

    private static final String FLOAT_REGEX = "(\\-)?\\d+\\.\\d+";
    private static final String INT_REGEX = "((\\-)?(0b[01]+)|((\\-)?0x[0-9a-f]+)|((\\-)?[0-9]+))[SL]?";
    
    private static boolean isInt(String str) {
        return str.matches(INT_REGEX);
    }
    
    private static boolean isFloat(String str) {
        return str.matches(FLOAT_REGEX);
    }
    
    public static Token genToken(String str) {
        if(isInt(str)) {
            return new IntToken(str);
        } else if(isFloat(str)) {
            return new FloatToken(str);
        } else {
            return new StringToken(str);
        }
    }
    
    public abstract Number getValue();
}
