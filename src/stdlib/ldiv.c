#include <stdlib.h>
/*

*/
ldiv_t ldiv(long numerator, long denominator)
{
	ldiv_t ret;
	lldiv_t ret_ = lldiv(numerator, denominator);

	ret.quot = ret_.quot;
	ret.rem = ret_.rem;

	return ret;
}
