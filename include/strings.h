/*
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)strings.h	5.6 (Berkeley) 12/12/88
 */

/* Routines described in string(3) */
char	*strcat(), *strncat(), *strcpy(), *strncpy(), *index(), *rindex();
int	strcmp(), strncmp(), strcasecmp(), strncasecmp(), strlen();

/* Routines described in memory(BA_LIB); System V compatibility */
char	*memccpy(), *memchr(), *memcpy(), *memset(), *strchr(),
	*strdup(), *strpbrk(), *strrchr(), *strsep(), *strtok();
int	memcmp(), strcspn(), strspn();

/* Routines from ANSI X3J11 */
char	*strerror();
