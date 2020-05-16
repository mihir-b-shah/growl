
package regex;

import utils.vector.IntStack;
import utils.vector.SmallArray;
import utils.queue.IntQueue;
import java.util.Arrays;
import java.util.PriorityQueue;
import java.util.TreeSet;

public class Regex {
    
    private static final int CHAR_SHIFT = 8;
    private static final int CHAR_MASK = 0xff;
    private static final char EPS_TRANSITION = '\0';
    
    /**
     * Growth factor 1.5, dynamic list.
     */
    static class NFAPool implements Comparable<NFAPool> {
        private SmallArray[] nfas;
        private int ptr;
        
        public NFAPool() {
            nfas = new SmallArray[6];
        }
        
        public final int add(SmallArray nfa) {
            if(nfas.length == ptr) {
                SmallArray[] aux = new SmallArray[(int) (ptr*1.5)];
                System.arraycopy(nfas, 0, aux, 0, ptr);
                nfas = aux;
            }
            nfas[ptr] = nfa;
            return ptr++;
        }
        
        public final SmallArray get(int i) {
            return nfas[i];
        }
        
        public final int size() {
            return ptr;
        }

        public final void clear() {
            ptr = 0;
        }
        
        public final void copy(NFAPool nfas, int src) {
            System.arraycopy(nfas, 0, nfas.nfas, src, ptr);
        }
        
        @Override
        public String toString() {
            return Arrays.toString(nfas);
        }
        
        @Override
        public int compareTo(NFAPool other) {
            return nfas.length-other.nfas.length;
        }
    }

    public static final NFAPool genLinearAutomaton(String regex, int start, 
            int end, NFAPool nfas) {
        nfas.add(new SmallArray()); //initial nfa
        int currNFA = 0;
        char curr;
        
        IntStack positions = new IntStack(end-start);
        
        for(int pos = start; pos<end; ++pos) {
            curr = regex.charAt(pos);
            switch(curr) {
                case '(':
                    positions.push(pos);
                    break;
                case ')':
                    int st = positions.pop()+1;
                    // from [st,pos) gen automaton
                    break;
                case '+':
                    nfas.get(currNFA).add(regex.charAt(pos-1) + (currNFA << CHAR_SHIFT));
                    break;
                case '?':
                    nfas.get(currNFA-1).add(EPS_TRANSITION + (currNFA << CHAR_SHIFT));
                    break;
                case '|':
                    break;
                default:
                    // concatenate with current NFA
                    SmallArray sm = new SmallArray();
                    int place = nfas.add(sm);
                    nfas.get(currNFA).add(curr + (place << CHAR_SHIFT));
                    currNFA = place;
                    break;
            }
        }
        
        return nfas;
    }
    
    public static final void genAutomaton(String regex) {
        IntStack stack = new IntStack();
        TreeSet<NFAPool> freeStorage = new TreeSet<>();

        int pos = 0;
        int startPos;
        char curr;
        
        while(pos < regex.length()) {
            curr = regex.charAt(pos);
            startPos = pos;
            
            while(curr != '(' && curr != ')') {
                ++pos;
            }
            NFAPool nfa = new NFAPool();
            genLinearAutomaton(regex, startPos, pos, nfa);
        }
        
        do {
            switch(regex.charAt(pos)) {
                case '(':
                    stack.push(pos);
                    break;
                case ')':
                    genLinearAutomaton(regex, stack.pop()+1, pos, null);
                    break;
                default:
                    
                    break;
            }
        } while (pos < regex.length());
    }
    
    public static final boolean check(String string, NFAPool nfa) {
        SmallArray curr;
        char ch;
        int packet;
        int currVal;
        
        IntQueue queue = new IntQueue(nfa.size());
        queue.push(0);

        while(queue.size() > 0) {
            packet = queue.pop();
            
            /* 
            System.out.printf("String pos: %d/%d, NFA pos: %d/%d%n", 
            packet >>> CHAR_SHIFT, string.length(), 
            packet & CHAR_MASK, nfa.size());
            */
            
            if(packet >= (string.length() << CHAR_SHIFT) + nfa.size()-1) {
                return true;
            }

            ch = packet >>> CHAR_SHIFT < string.length() 
                    ? string.charAt(packet >>> CHAR_SHIFT) : '\0';
            curr = nfa.get(packet & CHAR_MASK);
            
            for(int i = 0; i<curr.size(); ++i) {
                if(((currVal = curr.get(i)) & CHAR_MASK) == ch) {
                    queue.push((currVal >>> CHAR_SHIFT)
                                +(1+(packet >>> CHAR_SHIFT) << CHAR_SHIFT));
                } else if((currVal & CHAR_MASK) == 0) {
                    queue.push((currVal >>> CHAR_SHIFT)
                                +(packet >>> CHAR_SHIFT << CHAR_SHIFT));
                }
            }
        }
        return false;
    } 
    
    public static void main(String[] args) {
        String regex = "ab+cqe?";
        // NFAPool gen = genLinearAutomaton(regex, new NFAPool());
        // System.out.println(gen);
        // System.out.println(check("abbbbbcqee", gen));
    }
}
