
package parser.parsetypes.ast;

import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author mihir
 */
public class Switch extends ControlStr {

    public Primitive switchVal;
    public List<Literal> cases;
    public List<AST> jumps;

    public Switch() {
        cases = new ArrayList<>();
        jumps = new ArrayList<>();
    }
}