#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getw.c	5.2.1 (2.11BSD GTE) 1/1/94";
#endif

#include <stdio.h>

getw(iop)
register FILE *iop;
{
	register i;
	register char *p;
	int w;

	p = (char *)&w;
	for (i=sizeof(int); --i>=0;)
		*p++ = getc(iop);
	if (feof(iop))
		return(EOF);
	return(w);
}

#ifdef pdp11
long
getlw(iop)
register FILE *iop;
{
	register i;
	register char *p;
	long w;

	p = (char *)&w;
	for (i=sizeof(long); --i>=0;)
		*p++ = getc(iop);
	if (feof(iop))
		return(EOF);
	return(w);
}
#endif
