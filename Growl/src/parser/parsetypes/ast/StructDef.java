
package parser.parsetypes.ast;

import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author mihir
 */
public class StructDef extends UserDef {
        
    public String name;
    public List<Parameter> params = new ArrayList<>();
}
