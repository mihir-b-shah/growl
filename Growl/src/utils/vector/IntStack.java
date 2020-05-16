
package utils.vector;

/**
 * Primitive version of a stack.
 * 
 * @author mihir
 */
public class IntStack {
    private int[] stack;
    private int ptr;
    
    public IntStack() {
        stack = new int[2];
    }
    
    public IntStack(int maxSize) {
        stack = new int[maxSize];
    }
    
    public final void push(int v) {
        if(stack.length == ptr) {
            int[] aux = new int[ptr << 1];
            System.arraycopy(stack, 0, aux, 0, ptr);
            stack = aux;
        }
        stack[ptr++] = v;
    }
    
    public final int pop() {
        return stack[--ptr];
    }
    
    public final int size() {
        return ptr;
    }
}
