
package parser;

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.List;
import lexer.Lexeme;
import lexer.Lexer;
import parser.parsetypes.ast.Expression;

/**
 *
 * @author mihir
 */
public class ParseTests {
    public static void main(String[] args) throws Exception {
        BufferedReader br = new BufferedReader(
                new FileReader("parse_tests.txt"));

        String expr;
        String res;
        while((expr = br.readLine()) != null) {
            res = br.readLine();
            Lexer.initialize(expr);
            List<Lexeme> lexemes = Lexer.getInstance().getLexemes();
            Parser.initialize(lexemes);
            Expression expression = (Expression) Parser.getInstance().getAST();
            Number genRes = expression.evalConstExpr();
            if(genRes.getClass() == Long.class) {
                if((Long) genRes != Long.parseLong(res)) {
                    System.out.printf("TEST FAILED, expr: %s, expected: %s, actual: %d%n",
                            expr, res, genRes);
                } else {
                    System.out.printf("TEST OK, expr: %s%n", expr);
                }
            } else if(genRes.getClass() == Double.class) {
                if((Double) genRes != Double.parseDouble(res)) {
                    System.out.printf("TEST FAILED, expr: %s, expected: %s, actual: %d%n",
                            expr, res, genRes);
                } else {
                    System.out.printf("TEST OK, expr: %s%n", expr);
                }
            }
        }
        
        System.out.println("All tests complete.");
        br.close();
    }
}
