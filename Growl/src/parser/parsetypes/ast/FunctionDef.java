
package parser.parsetypes.ast;

import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author mihir
 */
public class FunctionDef extends UserDef {

    public Value returnVal;
    public AST body;
    public List<Parameter> parameters = new ArrayList<>();
}