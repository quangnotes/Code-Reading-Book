/* s_finitef.c -- float version of s_finite.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

#include <sys/cdefs.h>
#if defined(LIBM_SCCS) && !defined(lint)
__RCSID("$NetBSD: s_finitef.c,v 1.5 1997/10/09 11:31:56 lukem Exp $");
#endif

/*
 * finitef(x) returns 1 is x is finite, else 0;
 * no branching!
 */

#include "math.h"
#include "math_private.h"

#ifdef __STDC__
	int finitef(float x)
#else
	int finitef(x)
	float x;
#endif
{
	int32_t ix;
	GET_FLOAT_WORD(ix,x);
	return (int)((u_int32_t)((ix&0x7fffffff)-0x7f800000)>>31);
}