#include "facmath.h"
/** "private" function that computes the gcd of two numbers 
 * @param y
 * @param x
 * @return gcd of x and y
 */
long long gcd(long long x, long long y) {
	return (y != 0)? gcd(y, x%y) : x;
}
/**
 * take as input a fract and returns the normalized fract
 * @param f the fract to normalize
 * @return the normalized fract
 */
fract_t normalizeFract(fract_t f) {
	long long g = gcd(f.num, f.den);
	fract_t ret;
	ret.num = f.num/g;
	ret.den = f.den/g;
	return ret;
}
