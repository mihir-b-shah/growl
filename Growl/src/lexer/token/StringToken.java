
package lexer.token;

import parser.Parser;

/**
 * This class is not meant to be used. 
 * Its just to fill the other problems (not numbers).
 * 
 * @author mihir
 */
public class StringToken extends Token {
    public String token;
    
    public StringToken(String tok) {
        token = tok;
    } 
    
    @Override
    public Number getValue() {
        throw new Parser.ParseError();
    }
}
