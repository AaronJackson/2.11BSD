/* $Header: /home/hyperion/mu/christos/src/sys/tcsh-6.00/RCS/tc.decls.h,v 3.0 1991/07/04 23:34:26 christos Exp $ */
/*
 * tc.decls.h: Function declarations from all the tcsh modules
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
#ifndef _h_tc_decls
#define _h_tc_decls

/*
 * tc.alloc.c
 */
#ifndef SYSMALLOC
extern	void		  free		__P((ptr_t));
extern	memalign_t	  malloc	__P((size_t));
extern	memalign_t	  realloc	__P((ptr_t, size_t));
extern	memalign_t	  calloc	__P((size_t, size_t));

#else /* SYSMALLOC */
extern	void		  Free		__P((ptr_t));
extern	memalign_t	  Malloc	__P((size_t));
extern	memalign_t	  Realloc	__P((ptr_t, size_t));
extern	memalign_t	  Calloc	__P((size_t, size_t));
#endif /* SYSMALLOC */
extern	void		  showall	__P((void));

/*
 * tc.bind.c
 */
extern	void		  dobindkey	__P((Char **));
extern	int		  parseescape	__P((Char **));
extern	unsigned char    *u_p_string	__P((Char *, unsigned char *));
extern	void		  dobind	__P((Char **));


/*
 * tc.disc.c
 */
extern	int		  setdisc	__P((int));
extern	int		  resetdisc	__P((int));

/*
 * tc.func.c
 */
extern	Char		 *expand_lex	__P((Char *, int, struct wordent *, 
					     int, int));
extern	Char		 *sprlex	__P((Char *, struct wordent *));
extern	void		  Itoa		__P((int, Char *));
extern	void		  dolist	__P((Char **));
extern	void		  dotelltc	__P((Char **));
extern	void		  doechotc	__P((Char **));
extern	void		  dosettc	__P((Char **));
extern	void		  dowhich	__P((Char **));
extern	struct process	 *find_stop_ed	__P((void));
extern	void		  fg_proc_entry	__P((struct process *));
extern	sigret_t	  alrmcatch	__P((int));
extern	void		  precmd	__P((void));
extern	void		  cwd_cmd	__P((void));
extern	void		  period_cmd	__P((void));
extern	void		  aliasrun	__P((int, Char *, Char *));
extern	void		  setalarm	__P((void));
extern	void		  rmstar	__P((struct wordent *));
#ifdef pdp11
#define con_jobs cont_job
#endif
extern	void		  con_jobs	__P((struct wordent *));
extern	Char		 *gettilde	__P((Char *));
extern	Char		 *getusername	__P((Char **));
extern	void		  do_aliases	__P((Char **));


/*
 * tc.os.c
 */
#ifdef MACH
extern	void		  dosetpath	__P((Char **));
#endif
#ifdef TCF
extern	void		  dogetxvers	__P((Char **));
extern	void		  dosetxvers	__P((Char **));
extern	void		  dogetspath	__P((Char **));
extern	void		  dosetspath	__P((Char **));
extern	char		 *sitename	__P((pid_t));
extern	void		  domigrate	__P((Char **));
#endif
#ifdef WARP
extern	void 		  dowarp	__P((Char **));
#endif
#ifdef masscomp
extern	void		  douniverse	__P((Char **));
#endif
#ifdef _SEQUENT_
extern	void	 	  pr_stat_sub	__P((struct pro_stats *, 
					     struct pro_stats *, 
					     struct pro_stats *));
#endif
#ifdef NEEDtcgetpgrp
extern	int	 	  xtcgetpgrp	__P((int));
#define tcgetpgrp(a) xtcgetpgrp(a)
#endif
#ifdef YPBUGS
extern	void	 	  fix_yp_bugs	__P((void));
#endif
extern	void	 	  osinit	__P((void));
#ifdef NEEDgetwd
extern	char		 *xgetwd	__P((char *));
#define getwd(a) xgetwd(a)
#endif
#ifdef NEEDgethostname
extern	int	 	  gethostname	__P((char *, int));
#define gethostname(a, b) xgethostname(a, b)
#endif
#ifdef iconuxv
extern	int		  vfork		__P((void));
#endif
#ifdef apollo
extern	void		  doinlib	__P((Char **));
extern	void		  dover		__P((Char **));
extern	void		  dorootnode	__P((Char **));
extern	int		  getv		__P((Char *));
#endif


/*
 * tc.printf.h
 */
extern	void		  xprintf	__P((char *, ...));
extern	void		  mkprintf	__P((long, ...));
extern	void		  xsprintf	__P((char *, char *, ...));
extern	void		  xvprintf	__P((char *, va_list));
extern	void		  xvsprintf	__P((char *, char *, va_list));

/*
 * tc.prompt.c
 */
extern	void		  printprompt	__P((int, Char *));

/*
 * tc.sched.c
 */
extern	time_t		  sched_next	__P((void));
extern	void		  dosched	__P((Char **));
extern	void		  sched_run	__P((void));

/*
 * tc.sig.c
 */
#ifndef BSDSIGS
# if SVID < 3
#  ifdef UNIXPC
extern	sigret_t	(*sigset	__P((int, sigret_t (*)())) ();
extern	void		  sigrelse	__P((int));
extern	void		  sighold	__P((int));
extern	void		  sigignore	__P((int));
extern	void 		  sigpause	__P((int));
#  endif				/* UNIXPC */
extern	int 		  ourwait	__P((int *));
# endif
# ifdef SXA
extern	void 		  sigpause	__P((int));
# endif
#endif
#ifdef NEEDsignal
extern	sigret_t	(*xsignal	__P((int, sigret_t (*)()))) ();
#define signal(a, b)	  xsignal(a, b)
#endif
#ifdef _SEQUENT_
extern	sigmask_t	  sigsetmask	__P((sigmask_t));
extern	sigmask_t	  sigblock	__P((sigmask_t));
extern	void		  b_sigpause	__P((sigmask_t));
#endif
#ifdef SIGSYNCH
extern	sigret_t	  synch_handler	__P((int));
#endif




/*
 * tc.str.c:
 */
#ifdef SHORT_STRINGS
extern	Char		 *s_strchr	__P((Char *, int));
extern	Char		 *s_strrchr	__P((Char *, int));
extern	Char		 *s_strcat	__P((Char *, Char *));
#ifdef NOTUSED
extern	Char		 *s_scat	__P((Char *, Char *, size_t));
#endif
extern	Char		 *s_strcpy	__P((Char *, Char *));
extern	Char		 *s_scpy	__P((Char *, Char *, size_t));
extern	Char		 *s_strspl	__P((Char *, Char *));
extern	size_t		  s_strlen	__P((Char *));
extern	int		  s_strcmp	__P((Char *, Char *));
extern	int		  s_scmp	__P((Char *, Char *, size_t));
extern	Char		 *s_strsave	__P((Char *));
extern	Char		 *s_strend	__P((Char *));
extern	Char		 *s_strstr	__P((Char *, Char *));
extern	Char		 *str2short	__P((char *));
extern	Char		**blk2short	__P((char **));
extern	char		 *short2str	__P((Char *));
extern	char		 *short2qstr	__P((Char *));
extern	char		**short2blk	__P((Char **));
#endif


/*
 * tc.vers.h:
 */
extern	void		  fix_version	__P((void));
extern	Char		 *GetHostType	__P((void));

/*
 * tc.who.c
 */
extern	void		  initwatch	__P((void));
extern	void		  resetwatch	__P((void));
extern	void		  watch_login	__P((void));
extern	void		  dolog		__P((Char **));

#endif				/* _h_tc_decls */
