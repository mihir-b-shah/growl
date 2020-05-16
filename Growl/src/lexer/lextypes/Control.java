
package lexer.lextypes;

import lexer.Lexer;
import java.util.regex.Pattern;
import lexer.token.StringToken;
import lexer.token.Token;

public enum Control {
    IF, ELSE, GOTO, RETURN, WHILE,
    SWITCH, CASE, DEFAULT, BREAK;

    public static final Pattern CONTROL_REGEX = Pattern.compile("(else)|(goto)|(if)"
            + "|(return)|(void)|(while)|(switch)|(case)|(default)|(break)");

    public static Control createControl(Token s) {
        assert(s.getClass() == StringToken.class);
        switch(((StringToken) s).token) {
            case "if":
                return Control.IF;
            case "else":
                return Control.ELSE;
            case "goto":
                return Control.GOTO;
            case "return":
                return Control.RETURN;
            case "while":
                return Control.WHILE;
            case "switch":
                return Control.SWITCH;
            case "case":
                return Control.CASE;
            case "default":
                return Control.DEFAULT;
            case "break":
                return Control.BREAK;
            default:
                throw new Lexer.LexError();
        }
    }
}