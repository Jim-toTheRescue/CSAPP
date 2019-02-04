/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
	int ret = ~(~(x&~y)&~(~x&y));
	return ret;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
	int TMin = 0x1;
	TMin = TMin << 31;
	return TMin;
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
	int negated_x, xplus1, tmin, negated_1, ret;
	negated_x = ~x + 1;
	negated_1 = ~(0x0);
	tmin = negated_x + negated_1;
	xplus1 = x + 1; 
	ret = xplus1 ^ tmin; // if x == Tmax or x == -1 then ret = 0 and !ret = 1;
	return (!ret) ^ !(~x); //if x == -1 then !(~x) == 1 else if x == Tmax the !(~x) = 0
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
	int mask = 0xaa;
	int tmp = mask;
	int masked_x;
	mask = (tmp << 8) ^ mask;
	tmp = mask;
	mask = (tmp << 16) ^ mask;//mask = 0xaaaaaaaa
	masked_x = x & mask; //Only consider the odd bits.
	return !(mask^masked_x);
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
	x = ~x + 1;
	return x;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
	  int	upper28bits, lower4bits, uppermask, lowermask, lower4test, ret;
	  uppermask = ~(0x0f); // uppermask = 0xffffffc0
	  lowermask = 0xf;
	  upper28bits = uppermask & x;//supposedly upper28bits = 0x30;
	  lower4bits = lowermask & x; // cut out the lower4bits;
	  lower4test = !(lower4bits>>3) | !(lower4bits ^ 0x8) | !(lower4bits ^ 0x9);
	  ret = (upper28bits ^ 0x30) | !lower4test;
	  return !ret;
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
	int rety, retz, jz = ~0x0;
  jz += !x;//if x = 0 then jz = 0; if x != 0 then jz = -1(0xffffffff)
  rety = jz & y;
  retz = ~jz & z;
  return rety | retz;
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  //mayof means may overflow
	int negated_x = ~x + 1, ret, overflow_ret, xsign, ysign, sign, mayof;
	xsign = x >> 31;//arithmetic right shift
	ysign = y >> 31;
	// if mayof = 1 means overflow may occur, then take overflow_ret as result else take sign;
	mayof = xsign ^ ysign;
	overflow_ret = (!ysign) & xsign;
	ret = y + negated_x;//ret = y - x
 	sign = ret >> 31; // if x <= y, sign = 0 else sign = 1;
 	return (mayof & overflow_ret) | ((!mayof) & (!sign));
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
	int sign, reverse_abs_x, ret;
	sign = x >> 31;//arithmetic right shift. if x >= 0, sign = 0 else sign = 0xffffffff;
	reverse_abs_x = (~sign & (~x + 1)) | (sign & x);	// if x >= 0, reverse_abs_x = -x else x;
	ret = reverse_abs_x >> 31;	 
  	return ret + 1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) { 	
	return 0;
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
	unsigned mask = 0x7f800000, exp, frac, sign, ret;

	exp = (uf & mask) >> 23;
	frac = uf & 0x7fffff;
	sign = 0x80000000 & uf;
	if (exp == 0xff)//Infinity or NaN
		return uf;
	else if (exp == 0x0){//denormalization
		frac = frac << 1;
		if (frac > 0x7fffff)//if left shift causes overflow
			exp = (exp + 1) << 23;
		frac = frac & 0x7fffff;
	} else 
		exp = (exp + 1) << 23;
	ret = sign | exp | frac;
  	return ret;
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
	// ret = pow(-1, sign) * M * pow(2, E) = pow(-1, sign) * (1 + f) * pow(2, exp - Bias);
	int Bias = 127, exp, frac, sign, shift_cnt=0, sum=0, base, div, ret;
	sign = uf & 0x80000000;
	exp = (uf & 0x7f800000) >> 23;
	frac = uf & 0x7fffff;
	//exp == 0 or exp < Bias will make the result < 1, hence return 0;
	if (exp == 0 || exp < Bias) return 0;
	//exp == 0xff means Inf or NaN; exp - Bias > 31 will make the result out of the range of int;
	if (exp == 0xff || exp - Bias > 31) return 0x80000000;
	//normal situation: that's no overflow and larger than 1;
	base = 0x1 << (exp - Bias);// base is pow(2, E);
	ret = base;
	div = 1 << 23;//use div to calculate the f;
	while (frac != 0){
		if ((frac & 0x1) == 0x1)
			sum += (1 << shift_cnt);
		shift_cnt ++;
		frac >>= 1;
	}
	ret += base / div * sum;
	if (sign == 0x80000000)//negative
		ret = ~ret + 1;
	return ret;
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {// x = e - Bias
	int Bias = 127, MaxExp = 255, posINF = 0x7f800000, exp=0, frac = 0x1;
	unsigned ret;
	if (x > MaxExp - Bias) return posINF;//too large
	if ( x <= -Bias - 23 ) return 0x0;//too small
	// range that won't overflow;
	if (x > -127){//normalization
		frac = 0;
		exp = x + Bias;
		exp = exp << 23;
	} else //denormalization
		frac = frac << (149 + x); 
	ret = exp | frac;

	return ret;
}
