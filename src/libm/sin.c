/* origin: FreeBSD /usr/src/lib/msun/src/s_sin.c */
/*
 * ====================================================

 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/* sin(x)
 * Return sine function of x.
 *
 * kernel function:
 *      __sin_kernel	    ... sine function on [-pi/4,pi/4]
 *      __cos_kernel	    ... cose function on [-pi/4,pi/4]
 *      __rem_pio2       ... argument reduction routine
 *
 * Method.
 *      Let S,C and T denote the sin, cos and tan respectively on
 *      [-PI/4, +PI/4]. Reduce the argument x to y1+y2 = x-k*pi/2
 *      in [-pi/4 , +pi/4], and let n = k mod 4.
 *      We have
 *
 *	  n	sin(x)      cos(x)	tan(x)
 *     ----------------------------------------------------------
 *	  0	  S	   C	     T
 *	  1	  C	  -S	    -1/T
 *	  2	 -S	  -C	     T
 *	  3	 -C	   S	    -1/T
 *     ----------------------------------------------------------
 *
 * Special cases:
 *      Let trig be any of sin, cos, or tan.
 *      trig(+-INF)  is NaN, with signals;
 *      trig(NaN)    is that NaN;
 *
 * Accuracy:
 *      TRIG(x) returns trig(x) nearly rounded
 */

#include "libm.h"

double sin(double x)
{
	double y[2], z=0.0;
	int32_t n, ix;

	/* High word of x. */
	GET_HIGH_WORD(ix, x);

	/* |x| ~< pi/4 */
	ix &= 0x7fffffff;
	if (ix <= 0x3fe921fb) {
		if (ix < 0x3e500000) {  /* |x| < 2**-26 */
			/* raise inexact if x != 0 */
			if ((int)x == 0)
				return x;
		}
		return __sin_kernel(x, z, 0);
	}

	/* sin(Inf or NaN) is NaN */
	if (ix >= 0x7ff00000)
		return x - x;

	/* argument reduction needed */
	n = __rem_pio2(x, y);
	switch (n&3) {
	case 0: return  __sin_kernel(y[0], y[1], 1);
	case 1: return  __cos_kernel(y[0], y[1]);
	case 2: return -__sin_kernel(y[0], y[1], 1);
	default:
		return -__cos_kernel(y[0], y[1]);
	}
}
