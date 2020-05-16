
package utils.tree;

import java.util.Random;
import java.util.TreeSet;

/**
 * Testing framework for the FastTree implementation.
 * 
 * @author mihir
 */
public class TreeBenchmark {

}

/*
    private static IntTreeMap<IntWrapper> tree;
    private static TreeSet<IntWrapper> check;
    
    private static IntWrapper[] insert;
    private static Random rand;
    
    private static final int INSERT_SIZE = 9_900_000;
    private static final int QUERY_SIZE = 5_000_000;
   
    static class IntWrapper implementsComparable<IntWrapper> {

        private final int c;

        public IntWrapper(int c) {
            this.c = c;
        }

        @Override
        public String toString() {
            return "" + c;
        }
        
        @Override
        public int compareTo(IntWrapper other) {
            return value()-other.value();
        }
    }

    public static void main(String[] args) {
        tree = new IntTreeMap(0, new IntWrapper(0));
        check = new TreeSet<>();
        insert = new IntWrapper[INSERT_SIZE];
        rand = new Random();
        
        for(int i = 0; i<insert.length; ++i) {
            insert[i] = new IntWrapper(rand.nextInt());
        }
        
        int ctr = 0;
        for(IntWrapper iw: insert) {
            tree.insert(iw.value(), iw);
            check.add(iw);
            if(ctr % 100000 == 0) System.out.println(ctr/100000);
            ++ctr;
        }
        
        System.out.println("Reached tests.");
        IntWrapper test;
        for(int i = 0; i<QUERY_SIZE; ++i) {
            if(rand.nextBoolean()) {
                test = new IntWrapper(rand.nextInt());
            } else {
                test = insert[rand.nextInt(insert.length)];
            }
            /*
            if(tree.find(test) != null ^ check.contains(test)) {
                System.out.println("Accuracy error.");
                break;
            } 
        }
        
        for(int k = 100; k<100_000_000; k*=10) {
            TreeSet<IntWrapper> treeSet = new TreeSet<>();
            treeSet.add(new IntWrapper(0));
            FastTree<IntWrapper> fastTree = new FastTree<>(new IntWrapper(0));

            Random rand = new Random();
            IntWrapper[] insert = new IntWrapper[k];
            for(int i = 0; i<insert.length; ++i) {
                insert[i] = new IntWrapper(rand.nextInt());
            }

            long T = System.nanoTime();

            for(IntWrapper iw: insert) {
                treeSet.add(iw);
            }
            
            System.out.printf("TreeSet %d inserts: %d%n", k, System.nanoTime()-T);
           
            T = System.nanoTime();
            for(IntWrapper iw: insert) {
                treeSet.contains(iw);
            }
            System.out.printf("TreeSet %d contains: %d%n%n", k, System.nanoTime()-T);

            T = System.nanoTime();
            for(IntWrapper iw: insert) {
                fastTree.insert(iw);
            }
            System.out.printf("FastTree %d inserts: %d%n", k, System.nanoTime()-T);

            T = System.nanoTime();
            for(IntWrapper iw: insert) {
                fastTree.contains(iw);
            }
            System.out.printf("FastTree %d contains: %d%n%n%n", k, System.nanoTime()-T);
        }
        
    } */