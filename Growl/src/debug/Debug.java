
package debug;

/**
 * Debug constants.
 * 
 * @author mihir
 */
public class Debug {
    public static final int CONSOLE_WIDTH = 70;
    public static final String UNSUPPORTED = "";
    
    public static String genSpaceBlock(int len) {
        StringBuilder sb = new StringBuilder();
        for(int i = 0; i<len; ++i) {
            sb.append(' ');
        }
        return sb.toString();
    }
}
