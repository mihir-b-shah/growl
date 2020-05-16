
package lexer.lextypes;

import parser.Parser;
import parser.parsetypes.enums.*;
import lexer.Lexer;
import java.util.regex.Pattern;
import lexer.token.Token;
import lexer.token.StringToken;


/**
 * Operator enum.
 * 
 * @author mihir
 */
public enum Operator {
    INCREMENT("++"), DECREMENT("--"), BIT_NOT("~"), LOG_NOT("!"),
    ACCESS("."), MULTIPLY("*"), DEREFERENCE("$"), DIVIDE("/"), MODULUS("%"), ADD("+"),
    SUBTRACT("-"), BIT_LEFT("<<"), SBIT_RIGHT(">>"), UBIT_RIGHT(">>>"),
    LESS("<"), GREATER(">"), EQUAL("=="), ADDRESS("@"), BIT_AND("&"), BIT_OR("|"), 
    BIT_XOR("^"), LOG_AND("&&"), LOG_OR("||"), ASSIGN("=");
    
    String token;
    Operator(String s) {
        token = s;
    }
    
    @Override
    public String toString() {
        return token;
    }
    
    public static final int UNSUPPORTED = 0x7f7f7f7f;

    public static final Pattern OPERATOR_REGEX = Pattern.compile("[\\.\\[\\]!~/%\\*\\^]"
            + "|(\\+{1,2})|(\\-{1,2})|(<{1,2})|(>{1,3})|(\\&{1,2})|(\\"
            + "|{1,2})|(\\=){1,2}|@|\\$");

    // returns 0 for confused, 1 for unary, returns 2 for binary
    public OpType type() {
        switch(this) {
            case INCREMENT:
            case DECREMENT:
            case BIT_NOT:
            case LOG_NOT:
            case DEREFERENCE:
            case ADDRESS:
                return OpType.UNARY;
            default:
                return OpType.BINARY;
        }
    } 

    public int precedence() {
        switch(this) {
            case ACCESS:
            case DEREFERENCE:
            case ADDRESS:
                return 15;
            case INCREMENT:
            case DECREMENT:
                return 14;
            case LOG_NOT:
            case BIT_NOT:
                return 13;
            case MULTIPLY:
            case DIVIDE:
            case MODULUS:
                return 12;
            case ADD:
            case SUBTRACT:
                return 11;
            case BIT_LEFT:
            case SBIT_RIGHT:
            case UBIT_RIGHT:
                return 10;
            case LESS:
                return 9;
            case GREATER:
                return 8;
            case EQUAL:
                return 7;
            case BIT_AND:
                return 6;
            case BIT_XOR:
                return 5;
            case BIT_OR:
                return 4;
            case LOG_AND:
                return 3;
            case LOG_OR:
                return 2;
            case ASSIGN:
                return 1;
            default:
                return 0;
        }
    }

    public static boolean higher(Operator o1, Operator o2) {
        return o1.precedence() < o2.precedence();
    }

    public Associativity associate() {
        switch(this) {
            case INCREMENT:
            case DECREMENT:
            case LOG_NOT:
            case BIT_NOT:
            case LESS:
            case GREATER:
                return Associativity.NONE;
            case DEREFERENCE:
            case ASSIGN:
                return Associativity.RIGHT_TO_LEFT;
            default:
                return Associativity.LEFT_TO_RIGHT;
        }
    }

    public static Operator createOperator(Token s) {
        assert(s.getClass() == StringToken.class);
        switch(((StringToken) s).token) {
            case "++":
                return Operator.INCREMENT;
            case "--":
                return Operator.DECREMENT;
            case "~":
                return Operator.BIT_NOT;
            case "!":
                return Operator.LOG_NOT;
            case ".":
                return Operator.ACCESS;
            case "*":
                return Operator.MULTIPLY;
            case "/":
                return Operator.DIVIDE;
            case "%":
                return Operator.MODULUS;
            case "+":
                return Operator.ADD;
            case "-":
                return Operator.SUBTRACT;
            case "<<":
                return Operator.BIT_LEFT;
            case ">>":
                return Operator.SBIT_RIGHT;
            case ">>>":
                return Operator.UBIT_RIGHT;
            case "<":
                return Operator.LESS;
            case ">":
                return Operator.GREATER;
            case "==":
                return Operator.EQUAL;
            case "&":
                return Operator.BIT_AND;
            case "|":
                return Operator.BIT_OR;
            case "^":
                return Operator.BIT_XOR;
            case "&&":
                return Operator.LOG_AND;
            case "||":
                return Operator.LOG_OR;
            case "=":
                return Operator.ASSIGN;
            case "@":
                return Operator.ADDRESS;
            case "$":
                return Operator.DEREFERENCE;
            default:
                throw new Lexer.LexError();
        }
    }
    
    public Number evaluateUnary(Number a) {
        if(a.getClass() != Long.class) {
            throw new Parser.ParseError();
        }
        Long cast = (Long) a;
        switch(this) {
            case BIT_NOT:
                return ~cast;
            default:
                return UNSUPPORTED; 
        }
    }
    
    private Long evalLongOp(Long a, Long b) {
        Long ret;
        switch(this) {
            case MULTIPLY:
                ret = a*b;
                break;
            case DIVIDE:
                ret = a/b;
                break;
            case MODULUS:
                ret = a%b;
                break;
            case ADD:
                ret = a+b;
                break;
            case SUBTRACT:
                ret = a-b;
                break;
            case BIT_LEFT:
                ret = a << b;
                break;
            case SBIT_RIGHT:
                ret = a >> b;
                break;
            case UBIT_RIGHT:
                ret = a >>> b;
                break;
            case BIT_AND:
                ret = a&b;
                break;
            case BIT_XOR:
                ret = a^b;
                break;
            case BIT_OR:
                ret = a|b;
                break;
            default:
                throw new Parser.ParseError();
        }
        return ret;
    }
    
    private Double evalDoubleOp(Double a, Double b) {
        Double ret;
        switch(this) {
            case MULTIPLY:
                ret = a*b;
                break;
            case DIVIDE:
                ret = a/b;
                break;
            case MODULUS:
                ret = a%b;
                break;
            case ADD:
                ret = a+b;
                break;
            case SUBTRACT:
                ret = a-b;
                break;
            default:
                throw new Parser.ParseError();
        }
        return ret;
    }
    
    public Number evaluateBinary(Number aRaw, Number bRaw) {
        Number ret;
        switch(this) {
            case MULTIPLY:
            case DIVIDE:
            case MODULUS:
            case ADD:
            case SUBTRACT:
                if(aRaw.getClass() == Double.class && bRaw.getClass() == Double.class) {
                    return evalDoubleOp((Double) aRaw, (Double) bRaw);
                }
            case BIT_LEFT:
            case SBIT_RIGHT:
            case UBIT_RIGHT:
            case BIT_AND:
            case BIT_XOR:
            case BIT_OR:
                if(aRaw.getClass() == Long.class && bRaw.getClass() == Long.class) {
                    return evalLongOp((Long) aRaw, (Long) bRaw);
                } else throw new Parser.ParseError();
            default:
                return UNSUPPORTED;
        }
    }
}
