/*
 *
 * CSED 211, Fall 2022
 * DataLAb: Manipulating Bits
 * Assigned: 2022-09-07
 * Due: 2022-09-18 23:59 pm
 *
 * Namgyu Park (namgyu.park@postech.ac.kr) is the lead person for this assignment.
 *
*/


#if 0
LAB L1 INSTRUCTIONS:
#endif

/*
 *   #Homework1-1 
 *   bitNor - ~(x|y) using only ~ and & 
 *   Example: bitNor(0x6, 0x5) = 0xFFFFFFF8
 *   Legal ops: ~ &
 *   Max ops: 8
 *   Rating: 1
 */

int bitNor(int x, int y) {
  return ~x & ~y;
}


/*
 *   #Homework1-2
 *   isZero - returns 1 if x == 0, and 0 otherwise 
 *   Examples: isZero(5) = 0, isZero(0) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 2
 *   Rating: 1
 */
int isZero(int x) {
  //to be implemented
  return !x;
}


/* 
 *   #Homework1-3
 *   addOK - Determine if can compute x+y without overflow
 *   Example: addOK(0x80000000,0x80000000) = 0,
 *            addOK(0x80000000,0x70000000) = 1, 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int addOK(int x, int y) {
  int sum_sign = (x + y) >> 31; 
  int x_sign = x >> 31; 
  int y_sign = y >> 31; 

  return !((x_sign ^ sum_sign) & (y_sign ^ sum_sign));
}


/*   
 *   #Homework1-4
 *   absVal - absolute value of x
 *   Example: absVal(-1) = 1.
 *   You may assume -TMax <= x <= TMax
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 4
 */
int absVal(int x) {
  int x_sign = x >> 31; 
  return (x + x_sign) ^ x_sign;
}


/*   
 *   #Homework1-5
 *   logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
  int mask = ((1 << 31) >> n) << 1; 
  return (x >> n) & ~mask;
}
