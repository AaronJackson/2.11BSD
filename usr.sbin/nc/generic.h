/* generic.h -- anything you don't #undef at the end remains in effect.
   The ONLY things that go in here are generic indicator flags; it's up
   to your programs to declare and call things based on those flags.

   You should only need to make changes via a minimal system-specific section
   at the end of this file.  To build a new section, rip through this and
   check everything it mentions on your platform, and #undef that which needs
   it.  If you generate a system-specific section you didn't find in here,
   please mail me a copy so I can update the "master".

   I realize I'm probably inventing another pseudo-standard here, but
   goddamnit, everybody ELSE has already, and I can't include all of their
   hairball schemes too.  HAVE_xx conforms to the gnu/autoconf usage and
   seems to be the most common format.  In fact, I dug a lot of these out
   of autoconf and tried to common them all together using "stupidh" to
   collect data from platforms.

   In disgust...  _H*  940910, 941115, 950511.  Pseudo-version: 1.3

   Updated 951104 with many patches from netcat feedback, and properly
   closed a lot of slop in open-ended comments: version 1.4
   960217 + nextstep: version 1.5
*/

#ifndef GENERIC_H		/* only run through this once */
#define GENERIC_H

/* =============================== */
/* System calls, lib routines, etc */
/* =============================== */

/* How does your system declare malloc, void or char?  Usually void, but go
   ask the SunOS people why they had to be different... */
#define VOID_MALLOC

/* notably from fwtk/firewall.h: posix locking? */
#define HAVE_FLOCK		/* otherwise it's lockf() */

/* if you don't have setsid(), you might have setpgrp(). */
#define HAVE_SETSID

/* random() is generally considered better than rand() */
#define HAVE_RANDOM

/* the srand48/lrand48/etc family is s'posedly even better */
#define HAVE_RAND48
/* bmc@telebase and others have suggested these macros if a box *does* have
   rand48.  Will consider for later if we're doing something that really
   requires stronger random numbers, but netcat and such certainly doesn't.
#define srandom(seed) srand48((long) seed)
#define random()      lrand48() */

/* if your machine doesn't have lstat(), it should have stat() [dos...] */
#define HAVE_LSTAT

/* different kinds of term ioctls.  How to recognize them, very roughly:
   sysv/POSIX_ME_HARDER:  termio[s].h, struct termio[s], tty.c_*[]
   bsd/old stuff:  sgtty.h, ioctl(TIOCSETP), sgttyb.sg_*, tchars.t_*  */
#define HAVE_TERMIOS

/* dbm vs ndbm */
#define HAVE_NDBM

/* extended utmp/wtmp stuff.  MOST machines still do NOT have this SV-ism */
#define UTMPX

/* some systems have nice() which takes *relative* values... [resource.h] */
#define HAVE_SETPRIORITY

/* a sysvism, I think, but ... */
#define HAVE_SYSINFO

/* ============= */
/* Include files */
/* ============= */

/* Presence of these can be determined via a script that sniffs them
   out if you aren't sure.  See "stupidh"... */

/* stdlib comes with most modern compilers, but ya never know */
#define HAVE_STDLIB_H

/* not on a DOS box! */
#define HAVE_UNISTD_H

/* stdarg is a weird one */
#define HAVE_STDARG_H

/* dir.h or maybe ndir.h otherwise. */
#define HAVE_DIRENT_H

/* string or strings */
#define HAVE_STRINGS_H

/* if you don't have lastlog.h, what you want might be in login.h */
#define HAVE_LASTLOG_H

/* predefines for _PATH_various */
#define HAVE_PATHS_H

/* some SV-flavors break select stuff out separately */
#define HAVE_SELECT_H

/* assorted others */
#define HAVE_PARAM_H		/* in sys/ */
#define HAVE_SYSMACROS_H	/* in sys/ */
#define HAVE_TTYENT_H		/* securetty et al */

/* ======================== */
/* System-specific sections */
/* ======================== */

/* By turning OFF various bits of the above,  you can customize for
   a given platform.  Yes, we're ignoring the stock compiler predefines
   and using our own plugged in via the Makefile. */

/* Make some "generic" assumptions if all else fails */
#ifdef GENERIC
#undef HAVE_FLOCK
#if defined(SYSV) && (SYSV < 4)  /* TW leftover: old SV doesnt have symlinks */
#undef HAVE_LSTAT
#endif /* old SYSV */
#undef HAVE_TERMIOS
#undef UTMPX
#undef HAVE_PATHS_H
#undef HAVE_SELECT_H
#endif /* GENERIC */

#endif /* GENERIC_H */
