/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)getlog_.c	5.1.1 1997/10/2
 */

/*
 * get login name of user
 *
 * calling sequence:
 *	character*8 getlog, name
 *	name = getlog()
 * or
 *	call getlog (name)
 * where:
 *	name will receive the login name of the user, or all blanks if
 *	this is a detached process.
 */

#include <unistd.h>

getlog_(name, len)
char *name; long len;
{
	char *l = getlogin();

	b_char(l?l:" ", name, len);
}
