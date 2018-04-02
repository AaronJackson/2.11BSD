/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)setjmperr.c	5.2 (Berkeley) 3/9/86";
#endif LIBC_SCCS and not lint

#include <setjmp.h>			/* include any machine defines */

#define ERRMSG	"longjmp botch\n"

/*
 * This routine is called from longjmp() when an error occurs.
 * Programs that wish to exit gracefully from this error may
 * write their own versions.
 * If this routine returns, the program is aborted.
 */
longjmperror()
{

	write(2, ERRMSG, sizeof(ERRMSG));
}
