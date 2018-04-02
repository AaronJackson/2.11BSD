/*
 * Copyright (c) 1987 Regents of the University of California.
 * This file may be freely redistributed provided that this
 * notice remains attached.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getenv.c	5.4 (Berkeley) 3/13/87";
#endif LIBC_SCCS and not lint

#include <stdio.h>

/*
 * getenv(name) --
 *	Returns ptr to value associated with name, if any, else NULL.
 */
char *
getenv(name)
	char *name;
{
	int	offset;
	char	*_findenv();

	return(_findenv(name,&offset));
}

/*
 * _findenv(name,offset) --
 *	Returns pointer to value associated with name, if any, else NULL.
 *	Sets offset to be the offset of the name/value combination in the
 *	environmental array, for use by setenv(3) and unsetenv(3).
 *	Explicitly removes '=' in argument name.
 *
 *	This routine *should* be a static; don't use it.
 */
char *
_findenv(name,offset)
	register char *name;
	int	*offset;
{
	extern char	**environ;
	register int	len;
	register char	**P,
			*C;

	for (C = name,len = 0;*C && *C != '=';++C,++len);
	for (P = environ;*P;++P)
		if (!strncmp(*P,name,len))
			if (*(C = *P + len) == '=') {
				*offset = P - environ;
				return(++C);
			}
	return(NULL);
}
