
package utils.queue;

/**
 * Implements a version of FastQueue for primitives. Around 3x speedup.
 * 
 * @author mihir
 */

public class IntQueue {
    private int[] queue;
    private int front,back;
    private int size;
    private int MASK;

    public IntQueue(int N) {
        queue = new int[highPow2(N)];
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
    
    public final int pop() {
        --size;
        back &= MASK;
        return queue[back++];
    }
    
    public final int front() {
        return queue[back &= MASK];
    }
    
    public final void push(int val) {
        if(size > 0 && front == back) {
            int[] aux = new int[queue.length << 1];
            System.arraycopy(queue, back, aux, 0, queue.length-back);
            System.arraycopy(queue, 0, aux, queue.length-back, front);
            queue = aux;
            MASK <<= 1; MASK += 1;
            back = 0;
            front = queue.length >>> 1;
        }
        front &= MASK;
        ++size;
        queue[front++] = val;
    }
    
    public final int size() {
        return size;
    }
}
