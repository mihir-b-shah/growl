
import docutils.GroupFinder;
import docutils.SymbolTable;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.List;

import lexer.Lexeme;
import lexer.Lexer;
import parser.Parser;
import parser.parsetypes.ast.Expression;

public class Compiler {

    public static void main(String[] args) throws IOException {
        BufferedReader br = new BufferedReader(new FileReader("toyprog.txt"));
        StringBuilder sb = new StringBuilder();
        String line;
        while ((line = br.readLine()) != null) {
            sb.append(line);
            sb.append('\n');
        }

        String program = sb.toString();
        Lexer.initialize(program);
        List<Lexeme> lexemes = Lexer.getInstance().getLexemes();
        GroupFinder.initialize(lexemes);
        SymbolTable.initialize(lexemes);
        Parser.initialize(lexemes);
        Expression expr = (Expression) Parser.getInstance().getAST();
        System.out.println(expr);
    }
}
