/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)random_.c	5.3 (2.11BSD) 12/25/2018
 * 
 * Routines to return random values
 *
 * calling sequence:
 *	double precision d, drandm
 *	i = irandm(iflag)
 *	x = random(iflag)
 *	d = drandm(iflag)
 * where:
 *	If arg is nonzero, generator is restarted and value is returned.
 *	If arg is 0, next value is returned.
 *	Integer values will range from 0 thru 2147483647 (see random(3)).
 *	Real values will range from 0.0 thru 1.0 .
 */

#include <stdlib.h>

#if	vax
#define	RANDMAX		2147483647
#else	vax
#if	pdp11
#define	RANDMAX		2147483647
#else
	UNKNOWN MACHINE!
#endif	pdp11
#endif	vax

long irandm_(iarg)
long *iarg;
{
	if (*iarg) srandom(*iarg);
	return( random() );
}

float random_(iarg)
long *iarg;
{
	if (*iarg) srandom(*iarg);
	return( (float)(random())/(float)RANDMAX );
}

double drandm_(iarg)
long *iarg;
{
	if (*iarg) srandom(*iarg);
	return( (double)(random())/(double)RANDMAX );
}
