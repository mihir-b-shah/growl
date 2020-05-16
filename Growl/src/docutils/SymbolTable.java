
package docutils;

import lexer.lextypes.Group;
import lexer.*;

import java.util.HashMap;
import java.util.List;
import lexer.token.StringToken;

/**
 * Currently does not support functions.
 * Supoorts everything encapsulated by the DataType object.
 * 
 * @author mihir
 */
public class SymbolTable {
    private static SymbolTable instance = null;
    private final List<Lexeme> lexemes;
    private final HashMap<Symbol, Integer> table;
    
    class Symbol {
        int scope;
        String id;
        
        Symbol(int scope, String id) {
            this.scope = scope;
            this.id = id;
        }
        
        @Override
        public int hashCode() {
            return scope ^ id.hashCode();
        }

        @Override
        public boolean equals(Object obj) {
            if(obj instanceof Symbol) {
                Symbol loc = (Symbol) obj;
                return loc.scope == scope && loc.id.equals(id);
            } else return false;
        }
        
        @Override
        public String toString() {
            return id;
        }
    }
    
    @Override
    public String toString() {
        return table.toString();
    }
    
    private SymbolTable(List<Lexeme> lexemes) {
        this.lexemes = lexemes;
        table = new HashMap<>();
        findSymbols();
    }
    
    private void findSymbols() {
        Stack<Integer> stack = new Stack<>();
        stack.push(-1);
        
        for(int i = 0; i<lexemes.size(); ++i) {
            Lexeme lexeme = lexemes.get(i);
            switch(lexeme.type) {
                case ID:
                    Symbol sym = new Symbol(stack.peek(), ((StringToken) lexeme.token).token);
                    table.put(sym, i);
                    break;
                case GROUP:
                    Group gr = (Group) lexeme.subType;
                    switch(gr){
                        case OPEN_BRACKET:
                            stack.push(i);
                            break;
                        case CLOSE_BRACKET:
                            stack.pop();
                            break;
                        case OPEN_PAREN:
                            if(i>1 && lexemes.get(i-1).type == LexType.ID && 
                                   lexemes.get(i-2).type == LexType.TYPE) {
                                stack.push(i);
                            }
                            break;
                        case CLOSE_PAREN:
                            if(!stack.empty() && lexemes.get(stack.peek()).type == LexType.GROUP
                                    && ((Group) lexemes.get(stack.peek()).subType) == Group.CLOSE_PAREN) {
                                stack.pop();  
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
    }
    
    public static void initialize(List<Lexeme> lexemes) {
        instance = new SymbolTable(lexemes);
    }
    
    public static SymbolTable getInstance() {
        return instance;
    }
    
    // if efficiency mattered i would unroll the class...
    public int findSymbol(String id, int level) {
        return table.get(new Symbol(level, id));
    }
}
