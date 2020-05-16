
package utils.queue;

/**
 * Implements a fast version of a queue for references
 * with an additional value
 * More space-efficient than ArrayDeque
 * 
 * @author mihir
 * @param <T> the type parameter
 */

public class FastQueue<T> implements Queue<T> {
    private T[] queue;
    private int front,back;
    private int size;
    private int MASK;

    public FastQueue(int N) {
        queue = (T[]) new Object[highPow2(N)];
        MASK = queue.length-1;
    }
    
    private static final int highPow2(int x) {
        x -= 1;
        x |= x >>> 1;
        x |= x >>> 2;
        x |= x >>> 4;
        x |= x >>> 8;
        return x+1;
    }
    
    public final T pop() {
        --size;
        back &= MASK;
        return queue[back++];
    }
    
    public final T front() {
        return queue[back &= MASK];
    }
    
    public final void push(T val) {
        if(size > 0 && front == back) {
            T[] aux = (T[]) new Object[queue.length << 1];
            System.arraycopy(queue, back, aux, 0, queue.length-back);
            System.arraycopy(queue, 0, aux, queue.length-back, front);
            queue = aux;
            MASK <<= 1; MASK += 1;
            back = 0;
            front = queue.length >>> 1;
        }
        ++size;
        queue[front++] = val;
        front &= MASK;
    }
    
    public final int size() {
        return size;
    }
}
