
package utils.queue;

/**
 * Defines a simple queue without the Collection clutter.
 * 
 * @author mihir
 * @param <T> the type parameter.
 */
public interface Queue<T> {
    T pop();
    T front();
    void push(T val);
    int size();
}
