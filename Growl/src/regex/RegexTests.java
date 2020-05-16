
package regex;

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.Iterator;
import regex.Regex.NFAPool;
import utils.buffer.StringUtils;

/**
 *
 * @author mihir
 */
class RegexTests {
    
    static int LEVEL_TEST = 1;
    
    public static void main(String[] args) throws Exception {
        BufferedReader br = new BufferedReader(
                new FileReader("regex_tests.txt"));
        String line;
        String regex;
        String buf1,buf2;
        boolean res;
        boolean flag = false;
        NFAPool nfa;
        Iterator<String> st;

        while((line = br.readLine()) != null) {
            // this is so troll, using regex in a regex tester
            if(line.matches("\\d+") && Integer.parseInt(line) > LEVEL_TEST) {
                break;
            } else if(line.matches("\\d+")) {
                line = br.readLine();
            }
            
            st = StringUtils.tokenize(line, '\t');
            regex = st.next();
            // nfa = Regex.genAutomaton(regex);
            
            while(st.hasNext()) {
                if((res = Regex.check(buf1 = st.next(), null))  // fix
                        != ((buf2 = st.next()).charAt(0) == '1')) {
                    System.err.printf("Failed regex: %s, string %s. Should be "
                            + "%b, matched as %b.%n", regex, buf1, 
                            buf2.charAt(0)-'0' == 1, res);
                    flag = true;
                }
            }
            
            if(!flag) {
                System.out.printf("Regex %s passed all tests.%n", regex);
            }
        }
        
        System.out.println("All tests complete.");
        br.close();
    }
}
