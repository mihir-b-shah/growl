package parser;

import parser.parsetypes.ast.*;
import lexer.lextypes.*;
import docutils.Stack;
import lexer.Lexeme;
import docutils.GroupFinder;

import java.util.List;
import lexer.LexType;
import lexer.token.StringToken;
import parser.parsetypes.enums.Associativity;


/**
 * A recursive descent parser
 * 
 * @author mihir
 */
public class Parser {

    private final List<Lexeme> lexemes;
    private final AST syntaxTree;
    
    private static Parser instance = null;
    
    private Parser(List<Lexeme> lexes) {
        lexemes = lexes;
        syntaxTree = parse();
    }
    
    public static Parser getInstance() {
        return instance;
    }
    
    public AST getAST() {
        return syntaxTree;
    }
    
    public static void initialize(List<Lexeme> lexemes) {
        instance = new Parser(lexemes);
    }
    
    /*
    Currently scans for these control flow patterns:
    
    1] If-else statement (nested for now)
    
    Token stream: IF ( [expr] ) { [body] } ELSE { [body] }

    2] While loop
    
    Token stream: WHILE ( [expr] ) { [body] }
    
    3] Function definition
    
    Token stream: TYPE ID ( TYPE , ID , ... ) { [body] RETURN ID OR LITERAL}
    
    4] Labeled block
    
    Token stream: ID : { [body] }
    
    5] Struct definition
    
    Token stream: STRUCT ID { TYPE , ID ; TYPE , ID ;...};
    
    6] Switch block
    
    Token stream: SWITCH { case LITERAL : [comp]... } 
    
    7] Expression/Computation    
    */
    
    public static class ParseError extends Error {
        @Override
        public String getMessage() {
            return "Parse error encountered.";
        }
    }
    
    private AST parse() {
        return parseExpr(0, lexemes.size());
    }
    
    // inclusive, exclusive bounds
    private AST parseAST(int start, int end) {
        int parsePtr = start;
        while(parsePtr < end) {
            final Lexeme lex1 = lexemes.get(parsePtr);
            final Lexeme lex2;
            int match;
            AST result;
            int parenClose,brackClose;

            switch(lex1.type) {
                case ID:
                    if(start>end-2) {
                        break;
                    }
                    lex2 = lexemes.get(parsePtr+1);
                    if(lex2.subType.getClass() == Group.class
                            && ((Group) lex2.subType) == Group.COLON) {
                        // labeled block
                        Label block = new Label(); 
                        // correct type will parse until the curly brace open/close
                        match = GroupFinder.getInstance().findMatch(parsePtr+2);
                        result = parseAST(parsePtr+3, match);
                        block.block = result;
                        block.label = ((StringToken) lex1.token).token;
                        parsePtr = match+1;
                    }
                    break;
                case CONTROL:
                    switch((Control) lex1.subType) {
                        case IF:
                            // if/else statement
                            Branch block = new Branch(); 
                            // correct type will parse until the curly brace open/close
                            parenClose = GroupFinder.getInstance().findMatch(parsePtr+1);
                            Bool filter = new Bool();
                            filter.supplier = parseExpr(parsePtr+2, parenClose);
                            // curly brace next.
                            brackClose = GroupFinder.getInstance().findMatch(parenClose+1);
                            AST body = parseAST(parenClose+2, brackClose);
                            block.condition = filter;
                            block.ifBranch = body;
                            parsePtr = brackClose+1;
                            if(lexemes.get(parsePtr).type == LexType.CONTROL
                                && (((Control)(lexemes.get(parsePtr)).subType))
                                    == Control.ELSE) {
                                // continue parsing the else statement
                                brackClose = GroupFinder.getInstance().findMatch(parsePtr+1);
                                body = parseAST(parsePtr+2, brackClose);
                                block.elseBranch = body;
                                parsePtr = brackClose+1;
                            }
                            break;
                        case SWITCH:
                            // switch statement
                            parenClose = GroupFinder.getInstance().findMatch(parsePtr+1);
                            Switch switchBlock = new Switch();
                            Primitive switchExpr = 
                                    (Primitive) parseExpr(parsePtr+2, parenClose);
                            switchBlock.switchVal = switchExpr;
                            int switchPtr = parenClose+2;
                            int endPtr;
                            brackClose = GroupFinder.getInstance().findMatch(parenClose+1);
                            while(switchPtr < brackClose && 
                                    lexemes.get(switchPtr).subType == Control.class
                                 && ((Control) lexemes.get(switchPtr).subType) 
                                    == Control.CASE) {
                                Lexeme literal = lexemes.get(switchPtr+1);
                                switchBlock.cases.add(new Literal(literal));
                                endPtr = GroupFinder.getInstance().findNextCase(switchPtr+2, brackClose);
                                AST jump = parseAST(switchPtr+3, endPtr);
                                switchBlock.jumps.add(jump);
                                if(endPtr == brackClose) {
                                    break;
                                }
                                switchPtr = endPtr;
                            }
                            // check for default case
                            if(switchPtr < brackClose) {
                                AST jump = parseAST(switchPtr+2, brackClose);
                                switchBlock.jumps.add(jump);
                            }
                            break;
                        case WHILE:
                            // while statement
                            Loop whileBlock = new Loop(); 
                            // correct type will parse until the curly brace open/close
                            parenClose = GroupFinder.getInstance().findMatch(parsePtr+1);
                            Bool whileFilter = new Bool();
                            whileFilter.supplier = parseExpr(parsePtr+2, parenClose);
                            // curly brace next.
                            brackClose = GroupFinder.getInstance().findMatch(parenClose+1);
                            AST whileBody = parseAST(parenClose+2, brackClose);
                            whileBlock.condition = whileFilter;
                            whileBlock.body = whileBody;
                            parsePtr = brackClose+1;
                            break;
                    }
                case TYPE:
                    if(parsePtr>end-3) {
                        break;
                    }
                    lex2 = lexemes.get(parsePtr+1);
                    final Lexeme lex3 = lexemes.get(parsePtr+2);
                    if(lex2.subType.getClass() == Id.class 
                            && lex3.subType.getClass() == Group.class 
                            && ((Group) lex3.subType) == Group.OPEN_PAREN) {
                        // function definition
                        FunctionDef func = new FunctionDef();
                        int paramEnd = GroupFinder.getInstance().findMatch(parsePtr+3);
                        parsePtr += 4;
                        while(parsePtr < paramEnd) {
                            Parameter param = new Parameter();
                            param.type = (DataType) (lexemes.get(parsePtr).subType);
                            param.id = ((StringToken) lexemes.get(parsePtr+1).token).token;
                            func.parameters.add(param);
                            parsePtr += 2;
                        }
                        brackClose = GroupFinder.getInstance().findMatch(paramEnd+1);
                        AST funcBody = parseAST(paramEnd+2, brackClose);
                        func.body = funcBody;
                        parsePtr = brackClose+1;
                        // fix func.return() statement
                    } else if(((DataType) lex1.subType).base 
                            == DataType.BaseType.STRUCT) {
                        // struct declaration
                        StructDef struct = new StructDef();
                        struct.name = ((StringToken) lex1.token).token.substring(7);
                        brackClose = GroupFinder.getInstance().findMatch(parsePtr+1);
                        while(parsePtr < brackClose) {
                            Parameter param = new Parameter();
                            param.type = (DataType) (lexemes.get(parsePtr).subType);
                            param.id = ((StringToken) lexemes.get(parsePtr+1).token).token;
                            struct.params.add(param);
                            parsePtr += 2;
                        }
                        parsePtr = brackClose+1;
                    }
                    break;
            }
        }

        return null;
    }
    
    private int nextOperator(int ptr) {
        ++ptr;
        while(ptr < lexemes.size() && lexemes.get(ptr).type != LexType.OPERATOR) {
            ++ptr;
        }
        return ptr;
    }
    
    private void construct(Stack<Expression> output, Operator top) {
        Expression op1, op2;
        switch(top.type()) {
            case UNARY:
                op1 = output.pop();
                UnaryOp<Expression> uop = new UnaryOp<>();
                uop.operator = top;
                uop.op = op1;
                output.push(uop);
                break;
            case BINARY:
                op2 = output.pop();
                op1 = output.pop();
                BinaryOp<Expression> bop = new BinaryOp<>();
                bop.operator = top;
                bop.op1 = op1;
                bop.op2 = op2;
                output.push(bop);
                break;
            default:
                // parse error
                break;
        }
    } 
    
    // supports operators on NUMERICAL types.
    // so operands can be i8 (bool), i32, i64, f64
    // operators supported: (,),++,--!,~,*,/,%,+,-,<<,>>,>>>,<,>,==,&,|,^,&&,||
    // expression tree consists solely of operators
    private Expression parseExpr(int start, int end) {
        // stack can only contain groups and operators
        Stack<Lexeme> stack = new Stack<>();
        Stack<Expression> output = new Stack<>();
        
        // need to add functions
        // https://en.wikipedia.org/wiki/Shunting-yard_algorithm
        for(int ptr = start; ptr<end; ++ptr) {
            Lexeme lexeme = lexemes.get(ptr);
            switch(lexeme.type) {
                case ID:
                    Variable var = new Variable(lexeme);
                    output.push(var);
                case LITERAL:
                    Literal lit = new Literal(lexeme);
                    output.push(lit);
                    break;
                case OPERATOR:
                    Operator me = (Operator) lexeme.subType;
                    Operator top;
                    // watch out for associative
                    while(stack.size() > 0 && stack.peek().type == LexType.OPERATOR 
                            && ((top = (Operator) stack.peek().subType).precedence() 
                            > me.precedence() || top.precedence() == me.precedence() 
                            && me.associate() == Associativity.LEFT_TO_RIGHT)) {
                        stack.pop();
                        construct(output, top);
                    }
                    stack.push(lexeme); 
                    break;
                case GROUP:
                    switch((Group) lexeme.subType) {
                        case OPEN_PAREN:
                            stack.push(lexeme);
                            break;
                        case CLOSE_PAREN:
                            boolean flg = false;
                            while(stack.size() > 0 && (stack.peek().type != LexType.GROUP
                                    || ((Group) stack.peek().subType) != Group.OPEN_PAREN)) {
                                flg = true;
                                construct(output, (Operator) stack.pop().subType);
                            }
                            // only case, if hit paren
                            if(stack.size() > 0) {
                                stack.pop();
                            }
                            if(!flg) {
                                throw new ParseError();
                            }
                            break;
                        default:
                            throw new ParseError();
                    }
            }
        }
        while(stack.size() > 0) {
            construct(output, (Operator) stack.pop().subType);
        }
        assert(output.size() == 1);
        return output.pop();
    }
}