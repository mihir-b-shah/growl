
package utils.buffer;

import java.util.Iterator;

/**
 * Utility methods for strings.
 * 
 * @author mihir
 */
public class StringUtils {
    
    // just like StringTokenizer but supports zero-length matches.
    public static Iterator<String> tokenize(String str, char delim) {
        return new Iterator<String>(){
            private int pos;
            
            @Override
            public boolean hasNext() {
                return pos != str.length();
            }

            @Override
            public String next() {
                StringBuilder sb = new StringBuilder();
                char ch;
                while(pos < str.length() && (ch = str.charAt(pos++)) != delim) {
                    sb.append(ch);
                }
                return sb.toString();
            }
        };
    }
}
