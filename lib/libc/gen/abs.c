/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)abs.c	2.2 (Berkeley) 1/21/87";
#endif LIBC_SCCS and not lint

abs(arg)
	int arg;
{
	return(arg < 0 ? -arg : arg);
}
