
package lexer.lextypes;

import java.util.regex.Pattern;

/**
 * Class for id.
 * 
 * @author mihir
 */
public final class Id {
    public static final Pattern ID_REGEX
                = Pattern.compile("[\\w&&\\D][\\w]*");
}