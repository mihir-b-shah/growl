package docutils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import lexer.Lexeme;
import lexer.lextypes.Control;
import lexer.lextypes.Group;

/**
 * Find instances of puntctuation
 *
 * also add a O(lg n) lookup table.
 * 
 * @author mihir
 */
public class GroupFinder {
    private static GroupFinder instance = null;
    public static void initialize(List<Lexeme> document) {
        instance = new GroupFinder(document);
    }
    public static GroupFinder getInstance() {
        return instance;
    }
    
    private final List<Lexeme> document;
    private final ArrayList<Integer> positions;
    private final HashMap<Integer, Integer> lookup;

    private GroupFinder(List<Lexeme> doc) {
        document = doc;
        lookup = new HashMap<>();
        positions = new ArrayList<>();
        parseDoc();
    }
    
    private void parseDoc() {
        // note this is my stack not java.util.Stack
        // not optimizing this for primitives noooooooooo
        Stack<Integer> parenStack = new Stack<>();
        Stack<Integer> bracketStack = new Stack<>();

        int commaPos = -1;
        int semiPos = -1;
        int pos;
        
        KeywordFinder caseFinder = new KeywordFinder(Control.CASE, document);
        // not optimal, could use an arraylist with skip spots.
        HashMap<Integer, Integer> currCase = new HashMap<>();
        int casePos = -1;
        
        for(int i = 0; i<document.size(); ++i) {
            Lexeme lexeme = document.get(i);
            if(casePos < i && caseFinder.hasNext()) {
                casePos = caseFinder.next();
            } else if(casePos == i) {
                int lvl = bracketStack.size();
                Integer prCase = currCase.get(lvl);
                if(prCase == null) {
                    currCase.put(lvl, casePos);
                } else {
                    lookup.put(prCase, casePos);
                    currCase.put(lvl, casePos);
                }
            }
            if(lexeme.subType == null) {
                continue;
            }
            if(lexeme.subType.getClass() == Group.class) {
                switch((Group) lexeme.subType) {
                    case OPEN_PAREN:
                        parenStack.push(i);
                        break;
                    case CLOSE_PAREN:
                        pos = parenStack.pop();
                        lookup.put(pos, i);
                        break;
                    case OPEN_BRACKET:
                        bracketStack.push(i);
                        positions.add(i);
                        break;
                    case CLOSE_BRACKET:
                        pos = bracketStack.pop();
                        lookup.put(pos, i);
                        break;
                    case COMMA:
                        lookup.put(commaPos, i);
                        commaPos = i;
                        break;
                    case SEMICOLON:
                        lookup.put(semiPos, i);
                        semiPos = i;
                        break;
                }
            }
        }
        
        // remove the random key we inserted
        lookup.remove(-1);
    }
    

    // O(1) lookup on the point
    public int findMatch(int pos) {
        return lookup.get(pos);
    }
    
    // O(lg n) lookup within a block
    public int getBlock(int pos) {
        int ret = Collections.binarySearch(positions, pos);
        return ret < 0 ? ~ret-1 : ret == 0 ? -1 : ret;
    }
    
    public int findNextCase(int pos, int stop) {
        Integer ret = lookup.get(pos);
        return ret == null || ret >= stop ? stop : ret;
    }
    
    class Case {
        final int pos;
        final int lvl;
        
        Case(int p, int l) {
            pos = p;
            lvl = l;
        }
    }
    class KeywordFinder<T extends Enum> {
        
        final T find;
        final List<Lexeme> document;
        int pos;
        int ret; // mark for deletion
        
        KeywordFinder(T find, List<Lexeme> document) {
            this.find = find;
            this.document = document;
        }
        
        boolean hasNext() {
            while(pos < document.size() && (document.get(pos).subType == null
                    || document.get(pos).subType.getClass() != find.getClass() 
                    || ((T) document.get(pos).subType) != find)) {
                ++pos;
            }
            return pos == document.size();
        }
        
        int next() {
            return pos;
        }
    }
}