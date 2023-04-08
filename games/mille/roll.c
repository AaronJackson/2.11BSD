# include	"mille.h"
#include	<stdlib.h>

/*
 *	This routine rolls ndie nside-sided dice.
 *
 * @(#)roll.c	1.2 (2.11BSD) 2018/12/29
 *
 */

int roll(ndie, nsides)
reg int	ndie, nsides;
	{
	int	tot = 0;
	long 	r;

	while	(ndie--)
		{
		r = random();
		tot += r % nsides + 1;
		}
	return tot;
	}
