
package parser.parsetypes.ast;

/**
 *
 * @author mihir
 */
public class Branch extends ControlStr {

    public Bool condition;
    public AST ifBranch;
    public AST elseBranch;
}
