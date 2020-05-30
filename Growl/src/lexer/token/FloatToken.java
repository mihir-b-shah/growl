
package lexer.token;

/**
 * For floating point numbers
 * 
 * @author mihir
 */
public class FloatToken extends Token {
    public double token;
    
    public FloatToken(String tok) {
        token = Double.parseDouble(tok);
    }
    
    @Override
    public Double getValue() {
        return token;
    }
    
    @Override
    public String toString() {
        return Double.toString(token);
    }
}
