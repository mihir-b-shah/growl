
#include <vector>

/**
 * Implement the shunting yard algorithm.
 */


/*
// for now, only const expr
Expression parseExpr(int start, int end) {
    // stack can only contain groups and operators
    Stack<Lexeme> stack = new Stack<>();
    Stack<Expression> output = new Stack<>();
    
    // need to add functions
    // https://en.wikipedia.org/wiki/Shunting-yard_algorithm
    for(int ptr = start; ptr<end; ++ptr) {
        Lexeme lexeme = lexemes.get(ptr);
        switch(lexeme.type) {
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
*/