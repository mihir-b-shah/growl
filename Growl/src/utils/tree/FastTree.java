package utils.tree;

import java.io.PrintStream;
import java.util.Arrays;
import java.util.Random;
import java.util.TreeSet;
import java.util.function.ToIntFunction;
import java.util.stream.IntStream;
import utils.queue.FastQueue;

/**
 * Implements an optimized red-black binary search tree. The type of the input
 * is any input that can be expressed as an integer for comparison purposes.
 *
 * Most types can do this. All primitive wrappers can. Strings probably can't
 * unless you have some special size string.
 *
 * MAX CAPACITY OF 400 million nodes
 *
 * This can also be used as a multimap.
 *
 * Benchmarked as 1-1.5x faster for contains() and 1-2x faster for insert than
 * TreeMap. Based on current results, not slower for add/contains.
 *
 * The tree is stored as follows. Each node contains five fields:
 *
 * a KEY field that it a 32-bit integer The sign bit is whether RED OR BLACK The
 * rest is a reference to the data array.
 *
 * a VALUE field that is the absolute comparator value we need all the precision
 * we can get, so it is all 32 bits.
 *
 * a LEFT field that points to the left node.
 *
 * a RIGHT field that points to the right node.
 *
 * a PARENT field that points to the parent node
 * 
 * @author mihir
 * @param <T> the type.
 */
public class FastTree<T> implements Cloneable {

    private int[] tree; // a vector
    private int treePtr;

    private T[] data; // a vector
    private int dataPtr;

    private int rootPtr;
    private int size;

    private final ToIntFunction<T> compGenerator;

    private static final int HANDLE = 0;
    private static final int COMP = 1;
    private static final int LEFT = 2;
    private static final int RIGHT = 3;
    private static final int PARENT = 4;

    private static final int NULL = -1;
    private static final int BLK_SIZE = 5;

    private static final int RED = 1;
    private static final int BLACK = 0;
    private static final int TOGGLE = Integer.MIN_VALUE;
    private static final int SHIFT = 31;
    private static final int KEY_MASK = Integer.MAX_VALUE;

    private static final int FROM_LEFT = 0;
    private static final int FROM_RIGHT = 1;

    private static final String RED_COL = "\033[0;31m";
    private static final String RESET = "\033[0m";

    private static final int LEFT_LEFT = 0;
    private static final int LEFT_RIGHT = 1;
    private static final int RIGHT_LEFT = 2;
    private static final int RIGHT_RIGHT = 3;

    private static final int RED_RED = 3;
    private static final int RED_BLACK = 2;
    private static final int BLACK_RED = 1;
    private static final int BLACK_BLACK = 0;

    private static final int MAX_CAPACITY = 1 << 30;

    /**
     * Constructs a FastTree object.
     *
     * @param intFx the function giving the int compare value for a key.
     */
    public FastTree(ToIntFunction<T> intFx) {
        data = (T[]) new Object[4];
        tree = new int[20];
        compGenerator = intFx;
    }

    /**
     * Constructs a FastTree object.
     *
     * @param intFx the function giving the int compare value for a key.
     * @param capacity the capacity of the array
     */
    public FastTree(ToIntFunction<T> intFx, int capacity) {
        data = (T[]) new Object[capacity];
        tree = new int[BLK_SIZE * capacity];
        compGenerator = intFx;
    }

    /**
     * Does a pure left or right rotation.
     *
     * @param LR whether it is a left/right rotation. Left is true.
     * @param parentPtr the parent of <param>insertPtr</param>
     * @param pp the parent of <param>parentPtr</param>
     */
    private void pureRotate(boolean LR, int parentPtr, int pp) {
        final int LEFT, RIGHT;
        if (LR) {
            LEFT = FastTree.LEFT;
            RIGHT = FastTree.RIGHT;
        } else {
            RIGHT = FastTree.LEFT;
            LEFT = FastTree.RIGHT;
        }

        final int PP_PARENT = tree[pp + PARENT];
        if (PP_PARENT == -1) {
            rootPtr = parentPtr;
        } else {
            tree[(PP_PARENT & KEY_MASK) + 2 + (PP_PARENT >>> SHIFT)] = parentPtr;
        }

        final int T3 = tree[parentPtr + RIGHT];
        if (T3 != NULL) {
            tree[T3 + PARENT] = pp | LEFT - 2 << SHIFT;
        }

        tree[pp + PARENT] = parentPtr | RIGHT - 2 << SHIFT;
        tree[pp + LEFT] = T3;

        tree[parentPtr + PARENT] = PP_PARENT;
        tree[parentPtr + RIGHT] = pp;
    }

    /**
     * Does a mixed left or right rotation.
     *
     * @param LR whether it is a left/right rotation.
     * @param insertPtr the original insertion point.
     * @param parentPtr the parent of <param>insertPtr</param>
     * @param pp the parent of <param>parentPtr</param>
     */
    private void mixedRotate(boolean LR, int insertPtr, int parentPtr, int pp) {
        final int LEFT, RIGHT;
        if (LR) {
            LEFT = FastTree.LEFT;
            RIGHT = FastTree.RIGHT;
        } else {
            RIGHT = FastTree.LEFT;
            LEFT = FastTree.RIGHT;
        }

        final int PP_PARENT = tree[pp + PARENT];

        if (PP_PARENT == -1) {
            rootPtr = insertPtr;
        } else {
            tree[(PP_PARENT & KEY_MASK) + 2 + (PP_PARENT >>> SHIFT)] = insertPtr;
        }

        final int T2 = tree[insertPtr + LEFT];
        if (T2 != NULL) {
            tree[T2 + PARENT] = parentPtr | RIGHT - 2 << SHIFT;
        }
        final int T3 = tree[insertPtr + RIGHT];
        if (T3 != NULL) {
            tree[T3 + PARENT] = pp | LEFT - 2 << SHIFT;
        }

        // modify pp
        tree[pp + PARENT] = insertPtr | (RIGHT - 2) << SHIFT;
        tree[pp + LEFT] = T3;

        // modify parent
        tree[parentPtr + PARENT] = insertPtr | (LEFT - 2) << SHIFT;
        tree[parentPtr + RIGHT] = T2;

        // modfy insert
        tree[insertPtr + PARENT] = PP_PARENT;
        tree[insertPtr + LEFT] = parentPtr;
        tree[insertPtr + RIGHT] = pp;
    }

    /**
     * Rotates a 'balanced' configuration to the pure rotate input case,
     * essentially reverses the <code>pureRotate()</code> method.
     *
     * @param LR left/right config, left is true
     * @param highChild the higher child in the final position
     * @param sibling the sibling node.
     */
    private void reverseRotate(boolean LR, int highChild, int sibling) {
        final int LEFT, RIGHT;
        if (LR) {
            LEFT = FastTree.LEFT;
            RIGHT = FastTree.RIGHT;
        } else {
            RIGHT = FastTree.LEFT;
            LEFT = FastTree.RIGHT;
        }

        final int sibParent = tree[sibling + PARENT] & KEY_MASK;
        final int T2 = tree[highChild + RIGHT];
        if (T2 != NULL) {
            tree[T2 + PARENT] = sibling | LEFT - 2 << SHIFT;
        }

        tree[sibParent + RIGHT] = highChild;

        tree[sibling + PARENT] = highChild | RIGHT - 2 << SHIFT;
        tree[sibling + LEFT] = T2;

        tree[highChild + PARENT] = sibParent;
        tree[highChild + RIGHT] = sibling;
    }

    @Override
    public FastTree clone() throws CloneNotSupportedException {
        super.clone();
        FastTree<T> clTree = new FastTree<>(compGenerator, size);
        clTree.data = data;
        clTree.tree = this.tree;
        clTree.dataPtr = dataPtr;
        clTree.rootPtr = rootPtr;
        clTree.treePtr = treePtr;
        clTree.size = size;
        return clTree;
    }
    
    /**
     * Inserts an item into the BST.
     *
     * @param v the item to insert.
     * @return whether the item was inserted correctly.
     */
    public final boolean insert(final T v) {
        if (treePtr > MAX_CAPACITY - 1) {
            return false;
        } else if(size == 0) {
            data[HANDLE] = v;
            dataPtr = 1;
            treePtr = BLK_SIZE;
            tree[COMP] = compGenerator.applyAsInt(v);
            tree[LEFT] = NULL;
            tree[RIGHT] = NULL;
            tree[PARENT] = NULL;
            ++size;
        }
        // grow the array if needed
        if (dataPtr == data.length) {
            T[] aux = (T[]) new Object[dataPtr << 1];
            System.arraycopy(data, 0, aux, 0, dataPtr);
            data = aux;

            int[] aux2 = new int[treePtr << 1];
            System.arraycopy(tree, 0, aux2, 0, treePtr);
            tree = aux2;
        }

        // insert item into data table
        data[dataPtr] = v;

        int insertPtr = rootPtr;
        int parentPtr;
        int lrPtr;
        final int vComp = compGenerator.applyAsInt(v);

        int choosePtr;
        while (true) {
            if (vComp > tree[COMP + insertPtr]) {
                choosePtr = RIGHT;
            } else if (vComp < tree[COMP + insertPtr]) {
                choosePtr = LEFT;
            } else {
                // element already exists.
                return false;
            }
            if ((lrPtr = tree[insertPtr + choosePtr]) == NULL) {
                tree[insertPtr + choosePtr] = treePtr;
                tree[treePtr + HANDLE] = dataPtr ^ TOGGLE;
                tree[treePtr + COMP] = vComp;
                tree[treePtr + LEFT] = NULL;
                tree[treePtr + RIGHT] = NULL;
                tree[treePtr + PARENT] = insertPtr | (choosePtr - 2) << SHIFT;
                parentPtr = insertPtr;
                insertPtr = treePtr;
                break;
            } else {
                insertPtr = lrPtr;
            }
        }

        int sIndex;
        int auxVal;
        while (parentPtr > -1 && tree[parentPtr] >>> SHIFT == RED) {
            auxVal = tree[parentPtr + PARENT];
            sIndex = tree[(auxVal & KEY_MASK) + BLK_SIZE - 2 - (auxVal >>> SHIFT)];

            if (sIndex == NULL || tree[sIndex] >>> SHIFT == BLACK) {
                int pp = auxVal & KEY_MASK;
                // switch between the different rotation cases
                switch (((tree[parentPtr + PARENT] >>> SHIFT) << 1)
                        + (tree[insertPtr + PARENT] >>> SHIFT)) {
                    case LEFT_LEFT:
                        pureRotate(true, parentPtr, pp);
                        tree[pp] |= TOGGLE;
                        tree[parentPtr] &= KEY_MASK;
                        break;
                    case LEFT_RIGHT:
                        mixedRotate(true, insertPtr, parentPtr, pp);
                        tree[pp] |= TOGGLE;
                        tree[insertPtr] &= KEY_MASK;
                        break;
                    case RIGHT_LEFT:
                        mixedRotate(false, insertPtr, parentPtr, pp);
                        tree[pp] |= TOGGLE;
                        tree[insertPtr] &= KEY_MASK;
                        break;
                    case RIGHT_RIGHT:
                        pureRotate(false, parentPtr, pp);
                        tree[pp] |= TOGGLE;
                        tree[parentPtr] &= KEY_MASK;
                        break;
                }
                break;
            } else {
                tree[parentPtr] ^= TOGGLE; // toggle parent
                tree[sIndex] ^= TOGGLE; // toggle sibling
                if (auxVal != -1 && (auxVal &= KEY_MASK) != rootPtr) {
                    tree[auxVal] |= TOGGLE;
                    insertPtr = auxVal & KEY_MASK;
                    parentPtr = tree[auxVal + PARENT] & KEY_MASK;
                } else {
                    break;
                }
            }
        }

        ++dataPtr;
        treePtr += BLK_SIZE;
        ++size;
        return true;
    }

    /**
     * Finds the node in the tree.
     *
     * @param key the value.
     * @return <code>true</code> if found
     */
    public final boolean find(final T key) {
        long T = System.currentTimeMillis();
        final int vComp = compGenerator.applyAsInt(key);
        int currPtr = rootPtr;
        int currVal;

        while (true) {
            currVal = tree[currPtr + COMP];
            if (currPtr < 0) {
                return false;
            }
            if (vComp > currVal) {
                currPtr = tree[currPtr + RIGHT];
            } else if (vComp < currVal) {
                currPtr = tree[currPtr + LEFT];
            } else {
                return true;
            }
        }
    }
    
    /**
     * Removes the node from the tree.
     *
     * @param key the node
     * @return the node that was removed.
     */
    public final T erase(final T key) {
        int vComp = compGenerator.applyAsInt(key);
        int currPtr = rootPtr;
        int currVal;
        final int removePtr;

        // find the node.
        while (true) {
            currVal = tree[currPtr + COMP];
            if (currPtr < 0) {
                return null;
            }
            if (vComp > currVal) {
                currPtr = tree[currPtr + RIGHT];
            } else if (vComp < currVal) {
                currPtr = tree[currPtr + LEFT];
            } else {
                break;
            }
        }

        removePtr = currPtr;
        
        // keep finding inorder successors down the tree.
        inorder: {
            int ptr;
            if((ptr = tree[currPtr + RIGHT]) != NULL) {
                int prior;
                do {
                    prior = ptr;
                    ptr = tree[ptr + LEFT];
                } while (ptr != NULL);

                tree[currPtr] &= TOGGLE;
                tree[currPtr] |= tree[prior] & KEY_MASK;
                tree[currPtr + COMP] = tree[prior + COMP];
                currPtr = prior;
            }
        }
        
        // we have arrived at the case of leaf or right child only.
        final int nodeColor = tree[currPtr] >>> SHIFT;
        
        // getting which node is nonnull
        int childPtr, childLR;
        if(tree[currPtr + RIGHT] == -1) {
            childPtr = tree[currPtr + LEFT];
            childLR = FROM_LEFT;
        } else {
            childPtr = tree[currPtr + RIGHT];
            childLR = FROM_RIGHT;
        }

        // color of the child, accounts for null case
        final int childColor = childPtr == -1 ? 0 : tree[childPtr] >>> SHIFT;
        
        int auxPtr = -1;
        
        // pointer to the parent
        int parent = tree[currPtr + PARENT];
        
        // whether the parent is left/right on me
        final int pIndex = (parent & KEY_MASK) + 2 + (parent >>> SHIFT);
        //// System.out.println("parent: " + parent + "pIndex: " + pIndex);
        switch ((nodeColor << 1) + childColor) {
            case BLACK_RED:
                // // System.out.println("BLACK RED" + data[tree[currPtr]&KEY_MASK]);
                tree[childPtr] &= KEY_MASK;
            case RED_RED:
                // // System.out.println("RED RED" + data[tree[currPtr]&KEY_MASK]);
            case RED_BLACK:
                // // System.out.println("RED BLACK" + data[tree[currPtr]&KEY_MASK]);
                if(parent != -1) {
                    tree[pIndex] = childPtr;
                }
                if(childPtr != -1) {
                    tree[childPtr + PARENT] = tree[currPtr + PARENT] & TOGGLE | parent & KEY_MASK;
                }
                break;
            case BLACK_BLACK:
                final boolean nullModif;
                boolean skipTo5 = false;      
                int extraPtr = NULL;
                int pp = NULL;
                pp = tree[currPtr + PARENT] & KEY_MASK;
                if(pp != -1) {
                    pp = tree[pp + PARENT] & KEY_MASK;
                }
                        
                if(childPtr != -1) {
                    nullModif = false;
                    if(parent != -1) {
                        tree[pIndex] = childPtr;
                    }
                    tree[childPtr + PARENT] = tree[currPtr + PARENT] & TOGGLE | parent & KEY_MASK;
                } else {
                    nullModif = true;
                    // modify hierarchy
                    auxPtr = childPtr;
                    childPtr = currPtr;
                }
                // considers the reverse cases too
                // remedy for a goto statement
                // System.out.println(this.printTree(70));
                deleteCases:
                {
                    // case 1
                    if (parent == -1) {
                        // System.out.println("Case 1");
                        rootPtr = childPtr;
                        tree[childPtr + PARENT] = NULL;
                        break deleteCases;
                    }

                    // pointer to the parent
                    parent = tree[childPtr + PARENT];

                    // these are the sibling's left and right
                    int lrPtr = BLK_SIZE - 2 - (parent >>> SHIFT);
                    boolean lr = lrPtr == LEFT;
                    int sIndex = tree[(parent &= KEY_MASK) + lrPtr];
                    // // System.out.println("data[5]: " + data[5]);
                    boolean pass = false;
                    
                    /*
                    there are 7 variables that **might** need updating:
                    1. the double black node (childPtr)
                    2. its parent (parent)
                    3. the sibling (sIndex)
                    4. its left (left)
                    5. its right (right)
                    6. hasLeft
                    7. hasRight
                    */

                    // case 2
                    if (tree[parent] >>> SHIFT == BLACK
                            && sIndex != -1 && tree[sIndex] >>> SHIFT == RED) {
                        // System.out.println("Case 2");
                        pureRotate(lr, sIndex, parent);
                        tree[parent] |= TOGGLE;
                        tree[sIndex] &= KEY_MASK;
                        
                        /*
                        parent stays same
                        sibling index becomes prev lrPtr
                        */
                        sIndex = tree[parent + lrPtr];
                        pass = true;
                    }

                    // auxilliary
                    boolean hasLeft = false;
                    int left = sIndex == -1 ? -1 : tree[sIndex + LEFT];
                    if (sIndex != -1 && left != -1) {
                        hasLeft = true;
                    }
                    
                    boolean hasRight = false;
                    int right = sIndex == -1 ? -1 : tree[sIndex + RIGHT];
                    if (sIndex != -1 && right != -1) {
                        hasRight = true;
                    }

                    // case 3
                    if (tree[parent] >>> SHIFT == BLACK
                            && (sIndex != -1 && tree[sIndex] >>> SHIFT == BLACK)
                            && (!hasLeft || tree[left] >>> SHIFT == BLACK)
                            && (!hasRight || tree[right] >>> SHIFT == BLACK)) {
                        // System.out.println("Case 3");
                        if(hasLeft || hasRight) {
                            tree[sIndex] |= TOGGLE;
                        }                
                        // adjust ptrs
                        childPtr = parent & KEY_MASK;
                        parent = tree[childPtr + PARENT];
                        
                        if(parent != -1) {
                            lrPtr = BLK_SIZE - 2 - (parent >>> SHIFT);
                            sIndex = tree[(parent &= KEY_MASK) + lrPtr];
                            hasLeft = false;
                            left = tree[sIndex + LEFT];
                            if (left != -1) {
                                hasLeft = true;
                            }

                            hasRight = false;
                            right = tree[sIndex + RIGHT];
                            if (right != -1) {
                                hasRight = true;
                            }
                        }

                        pass = true;
                    } else if(tree[parent] >>> SHIFT == BLACK
                            && sIndex == -1) {
                        skipTo5 = true;
                        extraPtr = childPtr;
                        parent = pp;
                        
                        if(parent != -1) {
                            lrPtr = BLK_SIZE - 2 - (parent >>> SHIFT);
                            sIndex = tree[(parent &= KEY_MASK) + lrPtr];
                            hasLeft = false;
                            left = tree[sIndex + LEFT];
                            if (left != -1) {
                                hasLeft = true;
                            }

                            hasRight = false;
                            right = tree[sIndex + RIGHT];
                            if (right != -1) {
                                hasRight = true;
                            }
                        }
                    }
                    
                    // case 4
                    if (parent > -1 && tree[parent & KEY_MASK] >>> SHIFT == RED
                            && (sIndex == -1 || tree[sIndex] >>> SHIFT == BLACK)
                            && (!hasLeft || tree[left] >>> SHIFT == BLACK) 
                            && (!hasRight || tree[right] >>> SHIFT == BLACK)) {
                        // System.out.println("Case 4");
                        tree[parent] &= KEY_MASK;
                        if(sIndex > -1) {
                            tree[sIndex] |= TOGGLE;
                        }
                        pass = true;
                    }

                    if(pass && !skipTo5) {
                        break deleteCases;
                    }
                    
                    // case 5
                    if (sIndex != -1 && tree[sIndex] >>> SHIFT == BLACK) {
                        if (lrPtr == RIGHT && hasLeft && tree[left] >>> SHIFT == RED
                                && (!hasRight || tree[right] >>> SHIFT == BLACK)) {
                            // System.out.println("Case 5(1)");
                            reverseRotate(true, left & KEY_MASK, sIndex);
                            tree[left & KEY_MASK] &= KEY_MASK;
                            tree[sIndex] |= TOGGLE;

                            sIndex = tree[(parent &= KEY_MASK) + lrPtr];
                            hasLeft = false;
                            left = tree[sIndex + LEFT];
                            if (left != -1) {
                                hasLeft = true;
                            }

                            hasRight = false;
                            right = tree[sIndex + RIGHT];
                            if (right != -1) {
                                hasRight = true;
                            }
                            
                            pass = true;
                        } else if (lrPtr == LEFT && (!hasLeft || tree[left] >>> SHIFT == BLACK)
                                && hasRight && tree[right] >>> SHIFT == RED) {
                            // System.out.println("Case 5(2)");
                            reverseRotate(false, right & KEY_MASK, sIndex);
                            
                            tree[right & KEY_MASK] &= KEY_MASK;
                            tree[sIndex] |= TOGGLE;
                            
                            sIndex = tree[(parent &= KEY_MASK) + lrPtr];
                            hasLeft = false;
                            left = tree[sIndex + LEFT];
                            if (left != -1) {
                                hasLeft = true;
                            }

                            hasRight = false;
                            right = tree[sIndex + RIGHT];
                            if (right != -1) {
                                hasRight = true;
                            }
                            
                            pass = true;
                        }
                    }

                    // case 6
                    if (sIndex != -1 && tree[sIndex] >>> SHIFT == BLACK) {
                        // the left actually
                        if(lrPtr == RIGHT && hasRight && tree[right] >>> SHIFT == RED) {
                            // System.out.println("Case 6 (1)");
                            pureRotate(lr, sIndex, parent &= KEY_MASK);

                            int temp = tree[parent];
                            tree[parent] = (tree[sIndex] & TOGGLE) + (tree[parent] & KEY_MASK);
                            tree[sIndex] = (temp & TOGGLE) + (tree[sIndex] & KEY_MASK);
                            tree[tree[sIndex + RIGHT]] &= KEY_MASK;
                            pass = true;
                        } else if(lrPtr == LEFT && hasLeft && tree[left] >>> SHIFT == RED) {
                            // System.out.println("Case 6 (2)");
                            pureRotate(lr, sIndex, parent &= KEY_MASK);

                            int temp = tree[parent];
                            tree[parent] = (tree[sIndex] & TOGGLE) + (tree[parent] & KEY_MASK);
                            tree[sIndex] = (temp & TOGGLE) + (tree[sIndex] & KEY_MASK);
                            tree[tree[sIndex + LEFT]] &= KEY_MASK;
                            pass = true;
                        }
                        
                    }
                    
                    if(skipTo5) {
                        tree[extraPtr] |= TOGGLE; 
                    }
                    
                    if(pass) {
                        break deleteCases;
                    }
                }
                
                if(nullModif && pIndex > -1) {
                    tree[pIndex] = auxPtr;
                } 
                break;
        }
        --size;
        return null;
    }
    
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < treePtr; i += BLK_SIZE) {
            String el1 = data[(tree[i + HANDLE] & KEY_MASK)].toString();
            int comp = tree[i + COMP];
            String el2 = tree[i + LEFT] == -1 ? "NONE" : data[tree[tree[i + LEFT]] & KEY_MASK].toString();
            String el3 = tree[i + RIGHT] == -1 ? "NONE" : data[tree[tree[i + RIGHT]] & KEY_MASK].toString();
            String el4 = tree[i + HANDLE] >>> SHIFT == BLACK ? "BLACK" : "RED";
            String el5 = tree[i + PARENT] == -1 ? "NONE" : data[tree[(tree[i + PARENT] & KEY_MASK)] & KEY_MASK].toString();
            String el6 = tree[i + PARENT] >>> SHIFT == FROM_LEFT ? "LEFT" : "RIGHT";
            sb.append(String.format("Data: %s, Comp: %d, Left=%s, Right=%s, Color=%s, "
                    + "Parent=%s, Side=%s%n", el1, comp, el2, el3, el4, el5, el6));
        }
        return sb.toString();
    }

    /**
     * An encapsulation used by the printTree() method.
     */
    private class QueueItem {

        private final int bt;
        private final int height;
        private final int xJust;

        private QueueItem(int bt, int height, int xJust) {
            this.bt = bt;
            this.height = height;
            this.xJust = xJust;
        }
    }

    /**
     * Generate a graphical representation of the tree given a console width.
     * For small spaces relative to the console width, incorrect in-order
     * traversal can occur.
     *
     * @param consoleWidth the width of the print console
     * @return the string to print.
     */
    public final String printTree(int consoleWidth) {
        StringBuilder sb = new StringBuilder();
        FastQueue<QueueItem> queue = new FastQueue<>(8);
        queue.push(new QueueItem(rootPtr, 0, 0));

        int currHeight = -1;
        int currJustif = 0;

        while (queue.size() > 0) {
            QueueItem obj = queue.pop();

            if (currHeight != obj.height) {
                sb.append('\n');
                currHeight = obj.height;
                currJustif = 0;
            }

            int count = data[tree[obj.bt] & KEY_MASK].toString().length() >>> 1;
            int amt = (int) (consoleWidth * ((obj.xJust << 1) + 1)
                    / (1 << (obj.height + 1)));
            for (int i = 0; i < amt - currJustif - count; ++i) {
                sb.append(' ');
            }

            if (tree[obj.bt] >>> SHIFT == RED) {
                sb.append(RED_COL);
                sb.append(data[tree[obj.bt] & KEY_MASK].toString());
                sb.append(RESET);
            } else {
                sb.append(data[tree[obj.bt] & KEY_MASK].toString());
            }

            currJustif = amt;
            if (tree[LEFT + obj.bt] != NULL) {
                queue.push(new QueueItem(tree[LEFT + obj.bt],
                        obj.height + 1, 2 * obj.xJust + 0));
            }
            if (tree[RIGHT + obj.bt] != NULL) {
                queue.push(new QueueItem(tree[RIGHT + obj.bt],
                        obj.height + 1, 2 * obj.xJust + 1));
            }
        }

        return sb.toString();
    }
    
    public final int size() {
        return size;
    }

    public static void main(String[] args) {    
        /*
        FastTree<Integer> tree = new FastTree<>(Integer::intValue);
        int[] insert = {706, 94, 75, 472, 762, 178, 399, 160, 153, 297};
        int[] remove = {762, 399, 75, 472, 706};
        
        for(int e: insert) {
            tree.insert(e);
        }
        
        //tree.erase(3);
        System.out.println(tree.printTree(80));
        System.out.println(tree);
        
        for(int e: remove) {
            tree.erase(e);
            System.out.println("ELEMENT: " + e);
            System.out.println(tree.printTree(80));
            System.out.println(tree);
        }
        */
        
        for(int j = 0; j<10; ++j) {
            int ct = 0;
            for(int it = 0; it<100000; ++it) {
                FastTree<Integer> tree = new FastTree<>(Integer::intValue);
                TreeSet<Integer> correct = new TreeSet<>();

                Random rng = new Random();
                final int SIZE = 10;
                final int CONSOLE_WIDTH = 70;
                final int MAX_NUM = 1000;      

                int[] insert = IntStream.generate(()->rng.nextInt(MAX_NUM)).limit(SIZE).toArray();
                for(int e: insert) {
                    tree.insert(e);
                    correct.add(e);
                }

                int[] remove = new int[SIZE >>> 1];

                // // System.out.println(tree.printTree(CONSOLE_WIDTH));


                for(int i = 0; i<SIZE >>> 1; ++i) {
                    int el = insert[rng.nextInt(SIZE)];
                    remove[i] = el;
                    tree.erase(el);
                    correct.remove(el);


                    // System.out.println("ELEMENT: " + el);
                    // System.out.println(tree.printTree(CONSOLE_WIDTH));
                    // System.out.println(tree); 
                }

                for(int i = 0; i<SIZE; ++i) {
                    int el = insert[rng.nextInt(SIZE)];
                    if(tree.find(el) != correct.contains(el)) {
                        ++ct;
                        System.out.println(Arrays.toString(remove));
                    }
                }
            }

            System.out.println("ERROR: " + (double) ct/1000000);
        }
       
    }
}
