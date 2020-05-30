
package lexer.token;

/**
 * A type representable by an integer.
 * 
 * @author mihir
 */
public class IntToken extends Token {
    public long token;
    public int size;
    public boolean bool;
        
    public IntToken(String val) {
        switch(val.charAt(0)) {
            case 't':
                // bool::true token
                token = Limits.BOOL_TRUE;
                size = Limits.CHAR_SIZE;
                bool = true;
                break;
            case 'f':
                // bool::false token
                token = Limits.BOOL_FALSE;
                size = Limits.CHAR_SIZE;
                bool = true;
                break;
            case '\'':
                // character
                token = val.charAt(1);
                size = Limits.CHAR_SIZE;
                break;
            case '0':
                switch(val.charAt(1)) {
                    case 'b':
                        // binary
                        parse(val.substring(2), 2);
                        break;
                    case 'x':
                        // hex
                        parse(val.substring(2), 16);
                        break;
                    default:
                        // octal
                        parse(val.substring(1), 8);
                        break;
                }
                break;
            default:
                // decimal number
                parse(val.substring(0), 10);
                break;
        }
    }
    private void parse(String s, int radix) {
        switch(s.charAt(s.length()-1)) {
            case 'S':
                size = Limits.CHAR_SIZE;
                token = Long.parseLong(s.substring(0, s.length()-1), radix);
                break;
            case 'L':
                size = Limits.LONG_SIZE;
                token = Long.parseLong(s.substring(0, s.length()-1), radix);
                break;
            default:
                size = Limits.INT_SIZE;
                token = Long.parseLong(s, radix);
                break;
        }
    }
    
    @Override
    public Number getValue() {
        return token;
    }
    
    @Override
    public String toString() {
        return Long.toString(token);
    }
}
