// This class implements a sudoku grid
// It also provides methods for recursively solving the grid
// The algorithm used is a simple brute force
// Sudoku is a popular logic puzzle
// Written by Etienne Laurin - September 2006
// My first Java class that is more than 20 lines
public class Grid {
    private static final int LOCKED  = 0x200; // the value was found, don't modify it
    private static final int ALL     = 0x1FF; // all the value bits are set

    private int[] grid = new int[81];
    public boolean valid = true;
    public boolean verbose = false;

    public static void main(String[] args){
	int a = args.length - 1;
	if(args[a].length() != 81){
	    usage();
	    System.exit(-1);
	}
	Grid grid = new Grid(args[a]);
	if(a != 0)
	    grid.verbose = true;
	if(!grid.valid){
	    System.out.println("Invalid grid.");
	    grid.show();
	    System.exit(-1);
	}
	if(grid.solve()){
	    System.out.println("Final grid:");
	    grid.show();
	    System.exit(0);
	}else{
	    System.out.println("Cannot be solved.");
	    grid.show();
	}
	System.exit(-1);
    }

    // default constructor: empty grid (with all possibilities on every square)
    public Grid(){
	for(int i = 0; i < 81; i++){
	    grid[i] = ALL;
	}
    }

    // converts a 81 character string with '1' to '9' tokens into
    // our internal representation: the nth bit from the left is set
    // if the grid square might be able to receive he value n
    public Grid(String str){
	this();
	for(int i = 0; i < 81; i++){
	    char c = str.charAt(i);
	    if(c >= '1' && c <= '9'){
		if(!set(i, value(str.charAt(i) - '1')))
		    return;
	    }
	}
    }

    private Grid(int[] oldgrid){
	grid = oldgrid.clone();
    }

    // same as readgrid, but for individual characters. n ranges from 0 to 8
    private static int value(int n){
	return 1<<n;
    }

    // sets the position pos in grid to val, and adjust corresponding
    // row, column and block. return true if it matches, or set error_msg
    // and retyurn false otherwise
    public boolean set(int pos, int val){
	int i;
	int[] blck = block(pos); // see comment for block()
	int left; // used as lower bound in the row loop

	// rows
	left = pos/9*9; // integer arithmetic
	for(i = left; i < left + 9; i++)
	    if(i != pos && !unset(i,val))
		return valid = false;

	// columns
	for(i = pos % 9; i < 81; i+=9)
	    if(i != pos && !unset(i,val))
		return valid = false;

	// blocks
	for(i = 0; i < 9; i++)
	    if(blck[i] != pos && !unset(blck[i],val))
		return valid = false;

	// lock the grid value if it was set successfully
	grid[pos] = val | LOCKED;
	return true;
    }

    // removes val as a possibility for pos
    private boolean unset(int pos, int val){
	if(grid[pos] == val)
	    return valid = false;
	else grid[pos] &= ~val;
	return true;
    }

    private static int blocks[][] = new int[][] {
	{ 0, 1, 2, 9,10,11,18,19,20},
	{ 3, 4, 5,12,13,14,21,22,23},
	{ 6, 7, 8,15,16,17,24,25,26},
	{27,28,29,36,37,38,45,46,47},
	{30,31,32,39,40,41,48,49,50},
	{33,34,35,42,43,44,51,52,53},
	{54,55,56,63,64,65,72,73,74},
	{57,58,59,66,67,68,75,76,77},
	{60,61,62,69,70,71,78,79,80}
    };
    // returns an int[9] containing the position of the cells in the
    // same block as pos
    private static int[] block(int pos){
	// intuitive formula that passed complete black box testing
	int b = (pos/27)*3+(pos%9)/3;
	return blocks[b];
    }

    // attempts to solve the grid, return true if successfull
    public boolean solve(){
	int pos;
	int val;
	if(verbose){
	    System.out.println("current grid:");
	    show();
	}
	if(!locksingles())
	    return false;
	pos = nextpos();
	if(pos == -1)
	    return true;
	for(;;){
	    val = firstval(grid[pos]);
	    if(guess(pos,val))
		return true;
	    if(verbose)
		System.out.println("bad guess");
	    if(!unset(pos,val))
		return false;
	}
    }

    private boolean guess(int pos, int val){
	Grid g = copy();
	if(verbose)
	    System.out.println("guessing "+valchar(val)+" at "+pos);
	if(!g.set(pos,val)){
	    return false;
	}
	if(!g.solve())
	    return false;
	grid = g.grid;
	return true;
    }
    
    // finds unlocked squares with only one possible value
    // and attempts to set that value. returns false on failure
    private boolean locksingles(){
	int i;
	boolean loop;
	do{
	    loop = false;
	    for(i = 0; i < 81; i++){
		if(grid[i] == firstval(grid[i])){
		    loop = true;
		    if(!set(i,grid[i]))
			return false;
		}
	    }
	}while(loop);
	return true;
    }

    // find the grid square that has the less possibilities and return it
    // or return -1 if the grid is full of locked values
    int nextpos(){
	int pos;
	int size;
	int lastsize = 10;
	int lastpos = 0xdeadbeef; // The stupid java compiler complains when
	                          // I leave this variable uninitialised.
	for(pos = 0; pos < 81; pos++){
	    if(locked(grid[pos]))
		continue;
	    size = count(grid[pos]);
	    if(size < lastsize){
		lastsize = size;
		lastpos = pos;
	    }
	}
	if(lastsize == 10)
	    return -1;

	return lastpos;
    }


    // return the number of possibilities left in val
    private static int count(int val){
	int count = 0;
	val &= 0x1FF;
	do {
	    count += val & 1;
	    val >>= 1;
	}while(val != 0);
	return count;
    }

    // return the smallest numbered possibility in val
    private static int firstval(int val){
	int n;
	for(n = 1; n <= 0x100; n <<= 1){
	    if((n & val) != 0)
		return n;
	}
	return 0;
    }

    // returns 0 if val is unlocked
    private static boolean locked(int val){
	return (val & LOCKED) == LOCKED;
    }

    // attempts to provide user-friendly documentation
    public static void usage(){
	System.out.println("usage: solver [-v] <grid>  -- grid is a 81 character string");
    }

    // outputs the grid
    private void show(){
	int i;
	System.out.println("+-----------+");
	for(i = 0; i < 81; i++){
	    if(i % 3 == 0){
		if(i % 9 == 0 && i != 0) // don't ask me.. it just works
		    System.out.println('|');
		System.out.print('|');
	    }
	    System.out.print(locked(grid[i])?valchar(grid[i]):' ');
	}
	System.out.println("|\n+-----------+");
    }

    // returns printable representation of firstval(val), or '?' on error
    private static char valchar(int val){
	int n;
	for(n = 0; n < 9; n++){
	    if((val & (1 << n)) != 0)
		return (char)('1' + n);
	}
	return '?';
    }
    
    public Grid copy(){
	Grid g = new Grid(grid);
	g.verbose = verbose;
	g.valid = valid;
	return g;
    }
}
