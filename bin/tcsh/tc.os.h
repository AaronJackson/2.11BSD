/* $Header: /home/hyperion/mu/christos/src/sys/tcsh-6.00/RCS/tc.os.h,v 3.1 1991/07/05 19:07:33 christos Exp $ */
/*
 * tc.os.h: Shell os dependent defines
 */
/*-
 * Copyright (c) 1980, 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#ifndef _h_tc_os
#define _h_tc_os

#ifdef OREO
#include <sys/time.h>
#include <sys/resource.h>
#endif /* OREO */

#ifdef titan
extern int end;
#endif /* titan */

#ifdef hpux
# ifdef lint
/*
 * Hpux defines struct ucred, in <sys/user.h>, but if I include that
 * then I need to include the *world*
 * [all this to pass lint cleanly!!!]
 * so I define struct ucred here...
 */
struct ucred {
    int     foo;
};
# endif /* lint */

/*
 * hpux 7.0 does not define it
 */
# ifndef CSUSP
#  define CSUSP 032
# endif	/* CSUSP */

# include <sys/bsdtty.h>

# ifndef POSIX
#  ifdef BSDJOBS
#   define getpgrp(a) getpgrp2(a)
#   define setpgrp(a, b) setpgrp2(a, b)
#  endif /* BSDJOBS */
# endif	/* POSIX */
#endif /* hpux */

#ifdef HYPERCUBE
/*
 * for struct winsiz
 */
# include <sys/stream.h>
# include <sys/ptem.h>
# define NEEDgethostname
#endif /* HYPERCUBE */

#ifdef IRIS4D
# include <sys/time.h>
# include <sys/resource.h>
/*
 * BSDsetpgrp() and BSDgetpgrp() are BSD versions of setpgrp, etc.
 */
# define setpgrp BSDsetpgrp
# define getpgrp BSDgetpgrp
#endif /* IRIS4D */

/*
 * Stat
 */
#ifdef ISC
/* these are not defined for _POSIX_SOURCE under ISC 2.2 */
# ifndef S_IFMT
#  define S_IFMT  0170000		/* type of file */
#  define S_IFDIR 0040000		/* directory */
#  define S_IFCHR 0020000		/* character special */
#  define S_IFBLK 0060000		/* block special */
#  define S_IFREG 0100000		/* regular */
#  define S_IFIFO 0010000		/* fifo */
#  define S_IFNAM 0050000		/* special named file */
# endif /* S_IFMT */
#endif /* ISC */

#ifdef S_IFMT
# if !defined(S_ISDIR) && defined(S_IFDIR)
#  define S_ISDIR(a)	(((a) & S_IFMT) == S_IFDIR)
# endif	/* ! S_ISDIR && S_IFDIR */
# if !defined(S_ISCHR) && defined(S_IFCHR)
#  define S_ISCHR(a)	(((a) & S_IFMT) == S_IFCHR)
# endif /* ! S_ISCHR && S_IFCHR */
# if !defined(S_ISBLK) && defined(S_IFBLK)
#  define S_ISBLK(a)	(((a) & S_IFMT) == S_IFBLK)
# endif	/* ! S_ISBLK && S_IFBLK */
# if !defined(S_ISREG) && defined(S_IFREG)
#  define S_ISREG(a)	(((a) & S_IFMT) == S_IFREG)
# endif	/* ! S_ISREG && S_IFREG */
# if !defined(S_ISFIFO) && defined(S_IFIFO)
#  define S_ISFIFO(a)	(((a) & S_IFMT) == S_IFIFO)
# endif	/* ! S_ISFIFO && S_IFIFO */
# if !defined(S_ISNAM) && defined(S_IFNAM)
#  define S_ISNAM(a)	(((a) & S_IFMT) == S_IFNAM)
# endif	/* ! S_ISNAM && S_IFNAM */
# if !defined(S_ISNAM) && defined(S_IFNAM)
#  define S_ISNAM(a)	(((a) & S_IFMT) == S_IFNAM)
# endif	/* ! S_ISNAM && S_IFNAM */
# if !defined(S_ISLNK) && defined(S_IFLNK)
#  define S_ISLNK(a)	(((a) & S_IFMT) == S_IFLNK)
# endif	/* ! S_ISLNK && S_IFLNK */
# if !defined(S_ISSOCK) && defined(S_IFSOCK)
#  define S_ISSOCK(a)	(((a) & S_IFMT) == S_IFSOCK)
# endif	/* ! S_ISSOCK && S_IFSOCK */
#endif /* S_IFMT */

#ifndef S_IEXEC
# define S_IEXEC 0000100
#endif /* S_IEXEC */
#ifndef S_IXOTH
# define S_IXOTH (S_IEXEC >> 6)
#endif /* S_IXOTH */
#ifndef S_IXGRP
# define S_IXGRP (S_IEXEC >> 3)
#endif /* S_IXGRP */
#ifndef S_IXUSR
# define S_IXUSR S_IEXEC
#endif /* S_IXUSR */

/*
 * Access()
 */
#ifndef F_OK
# define F_OK 0
#endif /* F_OK */
#ifndef X_OK
# define X_OK 1
#endif /* X_OK */
#ifndef W_OK
# define W_OK 2
#endif /* W_OK */
#ifndef R_OK
# define R_OK 4
#endif /* R_OK */

/*
 * Open()
 */
#ifndef O_RDONLY
# define O_RDONLY	0
#endif /* O_RDONLY */
#ifndef O_WRONLY
# define O_WRONLY	1
#endif /* O_WRONLY */
#ifndef O_RDWR
# define O_RDWR		2
#endif /* O_RDWR */

/*
 * Lseek()
 */
#ifndef L_SET
# ifdef SEEK_SET
#  define L_SET		SEEK_SET
# else
#  define L_SET		0
# endif	/* SEEK_SET */
#endif /* L_SET */
#ifndef L_INCR
# ifdef SEEK_CUR
#  define L_INCR	SEEK_CUR
# else
#  define L_INCR	1
# endif	/* SEEK_CUR */
#endif /* L_INCR */
#ifndef L_XTND
# ifdef SEEK_END
#  define L_XTND	SEEK_END
# else
#  define L_XTND	2
# endif /* SEEK_END */
#endif /* L_XTND */

#ifdef _SEQUENT_
# define NEEDgethostname
#endif /* _SEQUENT_ */

#if defined(BSD) && defined(POSIXJOBS) 
# define setpgid(pid, pgrp)	setpgrp(pid, pgrp)
#endif /* BSD && POSIXJOBS */

#if defined(BSDJOBS) && !(defined(POSIX) && defined(POSIXJOBS))
# if !defined(_AIX370) && !defined(_AIXPS2)
#  define setpgid(pid, pgrp)	setpgrp(pid, pgrp)
# endif /* !_AIX370 && !_AIXPS2 */
# define tcsetpgrp(fd, pgrp)	ioctl((fd), TIOCSPGRP, (ioctl_t) &(pgrp))
# define NEEDtcgetpgrp
#endif /* BSDJOBS && !(POSIX && POSIXJOBS) */

#ifdef RENO
/*
 * Older versions of RENO had this broken. It is fixed now. 
 * In any case, we use ours...
 */
# define tcsetpgrp(fd, pgrp)	ioctl((fd), TIOCSPGRP, (ioctl_t) &(pgrp))
# define NEEDtcgetpgrp
#endif /* RENO */

#ifdef DGUX
# define setpgrp(a, b) setpgrp2(a, b)
# define getpgrp(a) getpgrp2(a)
#endif /* DGUX */

#ifdef SXA
# ifndef BSDNICE
/*
 * We check BSDNICE cause this is not defined in config.sxa.
 * Only needed in the system V environment.
 */
#  define setrlimit(a, b) 	b_setrlimit(a, b)
#  define getrlimit(a, b)	b_getrlimit(a, b)
# endif	/* BSDNICE */
# ifndef NOFILE
#  define	NOFILE	64
# endif	/* NOFILE */
#endif /* SXA */

#ifndef POSIX
# define mygetpgrp()    getpgrp(0)
#else /* POSIX */
# if defined(BSD) || defined(sun)
#  define mygetpgrp()    getpgrp(0)
# else /* BSD || sun */
#  define mygetpgrp()    getpgrp()
# endif	/* BSD || sun */
#endif /* POSIX */


#if SVID > 0 && !defined(OREO) && !defined(IRIS4D)
# define NEEDgetwd
#endif /* SVID > 0 && !OREO && !IRIS4D */

#ifndef S_IFLNK
# define lstat stat
#endif /* S_IFLNK */


#if defined(BSDTIMES) && !defined(_SEQUENT_)
typedef struct timeval tmval_t;
#endif /* BSDTIMES && ! _SEQUENT_ */

#ifdef NeXT
/*
 * From Tony_Mason@transarc.com, override NeXT's malloc stuff.
 */
# define malloc tcsh_malloc
# define calloc tcsh_calloc
# define realloc tcsh_realloc
# define free tcsh_free
#endif /* NeXT */


#if !defined(POSIX) || defined(sun)
extern time_t time();
extern char *getenv();
extern int atoi();
extern char *ttyname();

# ifndef hpux
extern int abort();
extern int qsort();
# else
extern void abort();
extern void qsort();
# endif
extern void perror();

# ifndef NEEDgethostname
extern int gethostname();
# endif

# ifdef pdp11
#  ifdef BUFSIZ
#   undef BUFSIZ
#  endif
#  define BUFSIZ 512
# endif

# ifdef BSDSIGS
#  if defined(_AIX370) || defined(MACH) || defined(NeXT) || defined(_AIXPS2)
extern int sigvec();
extern int sigpause();
#  else	/* _AIX370 || MACH || NeXT || _AIXPS2 */
extern sigret_t sigvec();
extern void sigpause();
#  endif /* _AIX370 || MACH || NeXT || _AIXPS2 */
#  ifndef pdp11
extern sigmask_t sigblock();
extern sigmask_t sigsetmask();
#  else
/* #define sigmask_t sigm_t in compat.h to avoid 7-char conflict with sigmask */
extern sigmask_t sigblock();
extern sigmask_t sigsetmask();
#  endif /* pdp11 */
# endif	/* BSDSIGS */

# ifndef killpg
extern int killpg();
# endif	/* killpg */

# ifndef lstat
extern int lstat();
# endif	/* lstat */

# ifdef SYSMALLOC
extern memalign_t malloc();
extern memalign_t realloc();
extern memalign_t calloc();
extern void free();
# endif	/* SYSMALLOC */

# ifdef BSDTIMES
extern int getrlimit();
extern int setrlimit();
extern int getrusage();
extern int gettimeofday();
# endif	/* BSDTIMES */

# if defined(NLS) && !defined(NOSTRCOLL) && !defined(NeXT)
extern int strcoll();
# endif

# ifdef BSDJOBS
#  ifdef BSDTIMES
extern int wait3();
#  else	/* ! BSDTIMES */
#   if !defined(POSIXJOBS) && !defined(_SEQUENT_)
extern int wait3();
#   else /* POSIXJOBS || _SEQUENT_ */
extern int waitpid();
#   endif /* POSIXJOBS || _SEQUENT_ */
#  endif /* ! BSDTIMES */
# else /* !BSDJOBS */
#  if SVID < 3
extern int ourwait();
#  else	/* SVID >= 3 */
extern int wait();
#  endif /* SVID >= 3 */
# endif	/* ! BSDJOBS */

# ifdef BSDNICE
extern int setpriority();
# else /* !BSDNICE */
extern int nice();
# endif	/* !BSDNICE */

#ifdef pdp11
extern int setpwent();
#else
extern void setpwent();
#endif
extern void endpwent();
extern struct passwd *getpwuid(), *getpwnam(), *getpwent();

# ifndef getwd
extern char *getwd();
# endif	/* getwd */
#else /* POSIX */

# if defined(sun) && !defined(__GNUC__)
extern char *getwd();
# endif	/* sun && ! __GNUC__ */

# ifdef RENO
extern void perror();		/* Reno declares that in stdio.h :-( */
# endif	/* RENO */
#endif /* POSIX */

#endif /* _h_tc_os */
