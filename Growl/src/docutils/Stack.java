package docutils;


// stack
public class Stack<T> {
    private T[] array;
    private int ctr;

    public Stack() {
        array = (T[]) new Object[10];
    }

    public void push(T obj) {
        if(ctr == array.length) {
            T[] aux = (T[]) new Object[ctr << 1];
            System.arraycopy(array, 0, aux, 0, ctr);
            array = aux;
        }
        array[ctr++] = obj;
    }

    public T pop() {
        return array[--ctr];
    }
    
    public T peek() {
        return array[ctr-1];
    }
    
    public int size() {
        return ctr;
    }
    
    public boolean empty() {
        return ctr == 0;
    }
}