
package utils.vector;

/**
 * Implements the small vector optimization (sort of)
 * Growth factor 1.5
 * 
 * @author mihir
 */
public class SmallArray {
    private short[] array;
    private long mask;
    private int ptr;
    
    private static final long EL_MASK = 0xffffL;
    
    private static final int EL4_SHIFT = 48;
    private static final int EL3_SHIFT = 32;
    private static final int EL2_SHIFT = 16;
    
    public SmallArray() {
        mask = 0L;
    }
    
    public SmallArray(int N) {
        if(N > 4) {
            array = new short[N];
        } else {
            mask = 0L;
        }
    }
    
    public final void add(long val) {
        switch(ptr) {
            case 0:
                mask = val;
                break;
            case 1:
                mask += val << EL2_SHIFT;
                break;
            case 2:
                mask += val << EL3_SHIFT;
                break;
            case 3:
                mask += val << EL4_SHIFT;
                break;
            case 4:
                array = new short[4];
                ptr = 0;
            default:
                if(ptr == array.length) {
                    short[] aux = new short[(int) (ptr*1.5)];
                    System.arraycopy(array, 0, aux, 0, ptr);
                    array = aux;
                }
                array[ptr] += val;     
                break;
        }
        ++ptr;
    }
    
    public final int get(int i) {
        switch(i) {
            case 0:
                return (int) (EL_MASK & mask);
            case 1:
                return (int) (EL_MASK & (mask >>> EL2_SHIFT));
            case 2:
                return (int) (EL_MASK & (mask >>> EL3_SHIFT));
            case 3:
                return (int) (EL_MASK & (mask >>> EL4_SHIFT));
            default:
                return array[i]; 
        }
    }
    
    public final int size() {
        return ptr;
    }
    
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        long loc;
        switch(ptr-1) {
            case 3:
                loc = EL_MASK & (mask >>> EL4_SHIFT);
                sb.append(loc >>> 8); sb.append(' ');
                sb.append(loc & 0xff); sb.append(' ');
                sb.append(' ');
            case 2:
                loc = EL_MASK & (mask >>> EL3_SHIFT);
                sb.append(loc >>> 8); sb.append(' ');
                sb.append(loc & 0xff); sb.append(' ');
                sb.append(' ');
            case 1:
                loc = EL_MASK & (mask >>> EL2_SHIFT);
                sb.append(loc >>> 8); sb.append(' ');
                sb.append(loc & 0xff); sb.append(' ');
                sb.append(' ');
            case 0:
                loc = EL_MASK & mask;
                sb.append(loc >>> 8); sb.append(' ');
                sb.append(loc & 0xff); sb.append(' ');
                return sb.toString();
            case -1:
                return "END";
            default:
                for(int i = 0; i<ptr; ++i) {
                    sb.append(array[i]); sb.append(' ');
                }
                return sb.toString().trim();
        }
    }
}
