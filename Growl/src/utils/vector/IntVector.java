
package utils.vector;

/**
 * Primitive version of a vector.
 * 
 * @author mihir
 */
public class IntVector {
    private int[] array;
    private int ptr;
    
    public IntVector() {
        array = new int[2];
    }
    
    public IntVector(int maxSize) {
        array = new int[maxSize];
    }
    
    public final void insert(int v) {
        if(array.length == ptr) {
            int[] aux = new int[ptr << 1];
            System.arraycopy(array, 0, aux, 0, ptr);
            array = aux;
        }
        array[ptr++] = v;
    }
    
    public final int get(int i) {
        return array[i];
    }
    
    public final int size() {
        return ptr;
    }
}
