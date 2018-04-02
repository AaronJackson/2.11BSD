/* $Header: /home/hyperion/mu/christos/src/sys/tcsh-6.00/RCS/sh.sem.c,v 3.0 1991/07/04 23:34:26 christos Exp $ */
/*
 * sh.sem.c: I/O redirections and job forking. A touchy issue!
 *	     Most stuff with builtins is incorrect
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
#include "config.h"
#if !defined(lint) && !defined(pdp11)
static char *rcsid() 
    { return "$Id: sh.sem.c,v 3.0 1991/07/04 23:34:26 christos Exp $"; }
#endif

#include "sh.h"

#ifdef FIOCLEX
#ifndef sun
#ifndef CLEX_DUPS
#define CLEX_DUPS
#endif				/* CLEX_DUPS */
#endif				/* sun */
#endif				/* FIOCLEX */

#ifdef sparc
#include <vfork.h>
#endif				/* sparc */

#ifdef VFORK
static	sigret_t	vffree	__P((int));
#endif
static	void		doio	__P((struct command *t, int *, int *));
static	void		chkclob	__P((char *));

/*
 * C shell
 */
/*VARARGS 1*/
void
execute(t, wanttty, pipein, pipeout)
    register struct command *t;
    int     wanttty, *pipein, *pipeout;
{
#if defined(convex) || defined(__convex__)
    extern bool use_fork;	/* use fork() instead of vfork()? */

#endif

    bool    forked = 0;
    struct biltins *bifunc;
    int     pid = 0;
    int     pv[2];

#ifdef BSDSIGS
    static sigmask_t csigmask;
# ifdef VFORK
    static sigmask_t ocsigmask;
# endif	/* VFORK */
#endif /* BSDSIGS */
#ifdef VFORK
    static int onosigchld = 0;
#endif /* VFORK */
    static int nosigchld = 0;

    if (t == 0)
	return;
    /*
     * From: Michael Schroeder <mlschroe@immd4.informatik.uni-erlangen.de>
     * Don't check for wantty > 0...
     */
    if (t->t_dflg & F_AMPERSAND)
	wanttty = 0;
    switch (t->t_dtyp) {

    case NODE_COMMAND:
	if ((t->t_dcom[0][0] & (QUOTE | TRIM)) == QUOTE)
	    (void) Strcpy(t->t_dcom[0], t->t_dcom[0] + 1);
	if ((t->t_dflg & F_REPEAT) == 0)
	    Dfix(t);		/* $ " ' \ */
	if (t->t_dcom[0] == 0)
	    return;
	/* fall into... */

    case NODE_PAREN:
	if (t->t_dflg & F_PIPEOUT)
	    mypipe(pipeout);
	/*
	 * Must do << early so parent will know where input pointer should be.
	 * If noexec then this is all we do.
	 */
	if (t->t_dflg & F_READ) {
	    (void) close(0);
	    heredoc(t->t_dlef);
	    if (noexec)
		(void) close(0);
	}
	if (noexec)
	    break;

	set(STRstatus, Strsave(STR0));

	/*
	 * This mess is the necessary kludge to handle the prefix builtins:
	 * nice, nohup, time.  These commands can also be used by themselves,
	 * and this is not handled here. This will also work when loops are
	 * parsed.
	 */
	while (t->t_dtyp == NODE_COMMAND)
	    if (eq(t->t_dcom[0], STRnice))
		if (t->t_dcom[1])
		    if (strchr("+-", t->t_dcom[1][0]))
			if (t->t_dcom[2]) {
			    setname("nice");
			    t->t_nice =
				getn(t->t_dcom[1]);
			    lshift(t->t_dcom, 2);
			    t->t_dflg |= F_NICE;
			}
			else
			    break;
		    else {
			t->t_nice = 4;
			lshift(t->t_dcom, 1);
			t->t_dflg |= F_NICE;
		    }
		else
		    break;
	    else if (eq(t->t_dcom[0], STRnohup))
		if (t->t_dcom[1]) {
		    t->t_dflg |= F_NOHUP;
		    lshift(t->t_dcom, 1);
		}
		else
		    break;
	    else if (eq(t->t_dcom[0], STRtime))
		if (t->t_dcom[1]) {
		    t->t_dflg |= F_TIME;
		    lshift(t->t_dcom, 1);
		}
		else
		    break;
#ifdef F_VER
	    else if (eq(t->t_dcom[0], STRver))
		if (t->t_dcom[1] && t->t_dcom[2]) {
		    setname("ver");
		    t->t_systype = getv(t->t_dcom[1]);
		    lshift(t->t_dcom, 2);
		    t->t_dflg |= F_VER;
		}
		else
		    break;
#endif
	    else
		break;

	/* is t a command */
	if (t->t_dtyp == NODE_COMMAND) {
	    /*
	     * Check if we have a builtin function and remember which one.
	     */
	    bifunc = isbfunc(t);
	}
	else {			/* not a command */
	    bifunc = NULL;
	}

	/*
	 * We fork only if we are timed, or are not the end of a parenthesized
	 * list and not a simple builtin function. Simple meaning one that is
	 * not pipedout, niced, nohupped, or &'d. It would be nice(?) to not
	 * fork in some of these cases.
	 */
	/*
	 * Prevent forking cd, pushd, popd, chdir cause this will cause the
	 * shell not to change dir!
	 */
	if (bifunc && (bifunc->bfunct == dochngd ||
		       bifunc->bfunct == dopushd ||
		       bifunc->bfunct == dopopd))
	    t->t_dflg &= ~(F_NICE);
	if (((t->t_dflg & F_TIME) || (t->t_dflg & F_NOFORK) == 0 &&
	     (!bifunc || t->t_dflg &
	      (F_PIPEOUT | F_AMPERSAND | F_NICE | F_NOHUP))) ||
	/*
	 * We have to fork for eval too.
	 */
	    (bifunc && (t->t_dflg & F_PIPEIN) != 0 &&
	     bifunc->bfunct == doeval))
#ifdef VFORK
	    if (t->t_dtyp == NODE_PAREN ||
		t->t_dflg & (F_REPEAT | F_AMPERSAND) || bifunc)
#endif
	    {
		forked++;
		/*
		 * We need to block SIGCHLD here, so that if the process does
		 * not die before we can set the process group
		 */
		if (wanttty >= 0 && !nosigchld) {
#ifdef BSDSIGS
		    csigmask = sigblock(sigmask(SIGCHLD));
#else
		    sighold(SIGCHLD);
#endif				/* BSDSIGS */

		    nosigchld = 1;
		}

		pid = pfork(t, wanttty);
		if (pid == 0 && nosigchld) {
#ifdef BSDSIGS
		    (void) sigsetmask(csigmask);
#else
		    (void) sigrelse(SIGCHLD);
#endif				/* BSDSIGS */
		    nosigchld = 0;
		}
	    }

#ifdef VFORK
	    else {
		int     ochild, osetintr, ohaderr, odidfds;
		int     oSHIN, oSHOUT, oSHDIAG, oOLDSTD, otpgrp;
		int     oisoutatty, oisdiagatty;

#ifndef FIOCLEX
		int     odidcch;

#endif
#ifdef BSDSIGS
		sigmask_t omask;
#endif				/* BSDSIGS */

		/*
		 * Prepare for the vfork by saving everything that the child
		 * corrupts before it exec's. Note that in some signal
		 * implementations which keep the signal info in user space
		 * (e.g. Sun's) it will also be necessary to save and restore
		 * the current sigvec's for the signals the child touches
		 * before it exec's.
		 */
#ifdef BSDSIGS

		/*
		 * Sooooo true... If this is a Sun, save the sigvec's. (Skip
		 * Gilbrech - 11/22/87)
		 */
#ifdef SAVESIGVEC
		sigvec_t savesv[NSIGSAVED];
		sigmask_t savesm;

#endif				/* SAVESIGVEC */
		if (wanttty >= 0 && !nosigchld && !noexec) {
#ifdef BSDSIGS
		    csigmask = sigblock(sigmask(SIGCHLD));
#else
		    sighold(SIGCHLD);
#endif				/* BSDSIGS */
		    nosigchld = 1;
		}
		omask = sigblock(sigmask(SIGCHLD) | sigmask(SIGINT));
#else
		(void) sighold(SIGCHLD);
		(void) sighold(SIGINT);
#endif
		ochild = child;
		osetintr = setintr;
		ohaderr = haderr;
		odidfds = didfds;
#ifndef FIOCLEX
		odidcch = didcch;
#endif
		oSHIN = SHIN;
		oSHOUT = SHOUT;
		oSHDIAG = SHDIAG;
		oOLDSTD = OLDSTD;
		otpgrp = tpgrp;
		oisoutatty = isoutatty;
		oisdiagatty = isdiagatty;
#ifdef BSDSIGS
		ocsigmask = csigmask;
#endif				/* BSDSIGS */
		onosigchld = nosigchld;
		Vsav = Vdp = 0;
		Vexpath = 0;
		Vt = 0;
#ifdef SAVESIGVEC
		savesm = savesigvec(savesv);
#endif				/* SAVESIGVEC */
#if defined(convex) || defined(__convex__)
		if (use_fork)
		    pid = fork();
		else
		    pid = vfork();
#else
		pid = vfork();
#endif

		if (pid < 0) {
#ifdef BSDSIGS
#ifdef SAVESIGVEC
		    restoresigvec(savesv, savesm);
#endif				/* SAVESIGVEC */
		    (void) sigsetmask(omask);
#else
		    (void) sigrelse(SIGCHLD);
		    (void) sigrelse(SIGINT);
#endif
		    stderror(ERR_NOPROC);
		}
		forked++;
		if (pid) {	/* parent */
#ifdef pdp11
                    register char **v;
#endif
#ifdef BSDSIGS
#ifdef SAVESIGVEC
		    restoresigvec(savesv, savesm);
#endif				/* SAVESIGVEC */
#endif				/* BSDSIGS */
		    child = ochild;
		    setintr = osetintr;
		    haderr = ohaderr;
		    didfds = odidfds;
		    SHIN = oSHIN;
#ifndef FIOCLEX
		    didcch = odidcch;
#endif
		    SHOUT = oSHOUT;
		    SHDIAG = oSHDIAG;
		    OLDSTD = oOLDSTD;
		    tpgrp = otpgrp;
		    isoutatty = oisoutatty;
		    isdiagatty = oisdiagatty;
#ifdef BSDSIGS
		    csigmask = ocsigmask;
#endif				/* BSDSIGS */
		    nosigchld = onosigchld;

		    xfree((ptr_t) Vsav);
		    Vsav = 0;
		    xfree((ptr_t) Vdp);
		    Vdp = 0;
		    xfree((ptr_t) Vexpath);
		    Vexpath = 0;
		    blkfree((Char **) Vt);
		    Vt = 0;
#ifdef pdp11
                    if (v = gargv)
                        gargv = 0, blkfree(v);
                    if (v = pargv)
                        pargv = 0, blkfree(v);
#endif
		    /* this is from pfork() */
		    palloc(pid, t);
#ifdef BSDSIGS
		    (void) sigsetmask(omask);
#else
		    (void) sigrelse(SIGCHLD);
		    (void) sigrelse(SIGINT);
#endif
		}
		else {		/* child */
		    /* this is from pfork() */
		    int     pgrp;
		    bool    ignint = 0;

		    if (nosigchld) {
#ifdef BSDSIGS
			(void) sigsetmask(csigmask);
#else
			(void) sigrelse(SIGCHLD);
#endif				/* BSDSIGS */
			nosigchld = 0;
		    }

		    if (setintr)
			ignint =
			    (tpgrp == -1 &&
			     (t->t_dflg & F_NOINTERRUPT))
			    || gointr && eq(gointr, STRminus);
		    pgrp = pcurrjob ? pcurrjob->p_jobid : getpid();
		    child++;
		    if (setintr) {
			setintr = 0;
#ifdef notdef
			(void) signal(SIGCHLD, SIG_DFL);
#endif
/*
 * casts made right for SunOS 4.0 by Douglas C. Schmidt
 * <schmidt%sunshine.ics.uci.edu@ROME.ICS.UCI.EDU>
 * (thanks! -- PWP)
 *
 * ignint ifs cleaned by Johan Widen <mcvax!osiris.sics.se!jw@uunet.UU.NET>
 * (thanks again)
 */
			if (ignint) {
			    (void) signal(SIGINT, SIG_IGN);
			    (void) signal(SIGQUIT, SIG_IGN);
			}
			else {
			    (void) signal(SIGINT,  vffree);
			    (void) signal(SIGQUIT, SIG_DFL);
			}

			if (wanttty >= 0) {
			    (void) signal(SIGTSTP, SIG_DFL);
			    (void) signal(SIGTTIN, SIG_DFL);
			    (void) signal(SIGTTOU, SIG_DFL);
			}

			(void) signal(SIGTERM, parterm);
		    }
		    else if (tpgrp == -1 &&
			     (t->t_dflg & F_NOINTERRUPT)) {
			(void) signal(SIGINT, SIG_IGN);
			(void) signal(SIGQUIT, SIG_IGN);
		    }

#ifdef _SEQUENT_
		    pgetty(wanttty ? wanttty : 1, pgrp);
#else				/* _SEQUENT_ */
		    pgetty(wanttty, pgrp);
#endif				/* _SEQUENT_ */

		    if (t->t_dflg & F_NOHUP)
			(void) signal(SIGHUP, SIG_IGN);
		    if (t->t_dflg & F_NICE)
#ifdef BSDNICE
			(void) setpriority(PRIO_PROCESS,
					   0, t->t_nice);
#else				/* BSDNICE */
			(void) nice(t->t_nice);
#endif				/* BSDNICE */
#ifdef F_VER
		    if (t->t_dflg & F_VER) {
			Setenv(STRSYSTYPE, t->t_systype ? STRbsd43 : STRsys53);
			dohash();
		    }
#endif
		}

	    }
#endif				/* VFORK */
	if (pid != 0) {
	    /*
	     * It would be better if we could wait for the whole job when we
	     * knew the last process had been started.  Pwait, in fact, does
	     * wait for the whole job anyway, but this test doesn't really
	     * express our intentions.
	     */
	    if (didfds == 0 && t->t_dflg & F_PIPEIN) {
		(void) close(pipein[0]);
		(void) close(pipein[1]);
	    }
	    if ((t->t_dflg & F_PIPEOUT) == 0) {
		if (nosigchld) {
#ifdef BSDSIGS
		    (void) sigsetmask(csigmask);
#else
		    (void) sigrelse(SIGCHLD);
#endif				/* BSDSIGS */
		    nosigchld = 0;
		}
		if ((t->t_dflg & F_AMPERSAND) == 0)
		    pwait();
	    }
	    break;
	}
	doio(t, pipein, pipeout);
	if (t->t_dflg & F_PIPEOUT) {
	    (void) close(pipeout[0]);
	    (void) close(pipeout[1]);
	}
	/*
	 * Perform a builtin function. If we are not forked, arrange for
	 * possible stopping
	 */
	if (bifunc) {
	    func(t, bifunc);
	    if (forked)
		exitstat();
	    break;
	}
	if (t->t_dtyp != NODE_PAREN) {
	    doexec(t);
	    /* NOTREACHED */
	}
	/*
	 * For () commands must put new 0,1,2 in FSH* and recurse
	 */
	OLDSTD = dcopy(0, FOLDSTD);
	SHOUT = dcopy(1, FSHOUT);
	isoutatty = isatty(SHOUT);
	SHDIAG = dcopy(2, FSHDIAG);
	isdiagatty = isatty(SHDIAG);
	(void) close(SHIN);
	SHIN = -1;
#ifndef FIOCLEX
	didcch = 0;
#endif
	didfds = 0;
	wanttty = -1;
	t->t_dspr->t_dflg |= t->t_dflg & F_NOINTERRUPT;
	execute(t->t_dspr, wanttty, NULL, NULL);
	exitstat();

    case NODE_PIPE:
	t->t_dcar->t_dflg |= F_PIPEOUT |
	    (t->t_dflg & (F_PIPEIN | F_AMPERSAND | F_STDERR | F_NOINTERRUPT));
	execute(t->t_dcar, wanttty, pipein, pv);
	t->t_dcdr->t_dflg |= F_PIPEIN | (t->t_dflg &
			(F_PIPEOUT | F_AMPERSAND | F_NOFORK | F_NOINTERRUPT));
	if (wanttty > 0)
	    wanttty = 0;	/* got tty already */
	execute(t->t_dcdr, wanttty, pv, pipeout);
	break;

    case NODE_LIST:
	if (t->t_dcar) {
	    t->t_dcar->t_dflg |= t->t_dflg & F_NOINTERRUPT;
	    execute(t->t_dcar, wanttty, NULL, NULL);
	    /*
	     * In strange case of A&B make a new job after A
	     */
	    if (t->t_dcar->t_dflg & F_AMPERSAND && t->t_dcdr &&
		(t->t_dcdr->t_dflg & F_AMPERSAND) == 0)
		pendjob();
	}
	if (t->t_dcdr) {
	    t->t_dcdr->t_dflg |= t->t_dflg &
		(F_NOFORK | F_NOINTERRUPT);
	    execute(t->t_dcdr, wanttty, NULL, NULL);
	}
	break;

    case NODE_OR:
    case NODE_AND:
	if (t->t_dcar) {
	    t->t_dcar->t_dflg |= t->t_dflg & F_NOINTERRUPT;
	    execute(t->t_dcar, wanttty, NULL, NULL);
	    if ((getn(value(STRstatus)) == 0) !=
		(t->t_dtyp == NODE_AND))
		return;
	}
	if (t->t_dcdr) {
	    t->t_dcdr->t_dflg |= t->t_dflg &
		(F_NOFORK | F_NOINTERRUPT);
	    execute(t->t_dcdr, wanttty, NULL, NULL);
	}
	break;
    }
    /*
     * Fall through for all breaks from switch
     * 
     * If there will be no more executions of this command, flush all file
     * descriptors. Places that turn on the F_REPEAT bit are responsible for
     * doing donefds after the last re-execution
     */
    if (didfds && !(t->t_dflg & F_REPEAT))
	donefds();
}

#ifdef VFORK
static sigret_t
/*ARGSUSED*/
vffree(snum)
int snum;
{
    register Char **v;

    if (v = gargv) {
	gargv = 0;
#ifdef pdp11
        blkfree(v);
#else
	xfree((ptr_t) v);
#endif
    }
    if (v = pargv) {
	pargv = 0;
#ifdef pdp11
        blkfree(v);
#else
	xfree((ptr_t) v);
#endif
    }
    _exit(1);
}

#endif

/*
 * Perform io redirection.
 * We may or maynot be forked here.
 */
static void
doio(t, pipein, pipeout)
    register struct command *t;
    int    *pipein, *pipeout;
{
    register int fd;
    register Char *cp, *dp;
    register int flags = t->t_dflg;

    if (didfds || (flags & F_REPEAT))
	return;
    if ((flags & F_READ) == 0) {/* F_READ already done */
	if (cp = t->t_dlef) {
	    char    tmp[MAXPATHLEN+1];

	    /*
	     * so < /dev/std{in,out,err} work
	     */
	    (void) dcopy(SHIN, 0);
	    (void) dcopy(SHOUT, 1);
	    (void) dcopy(SHDIAG, 2);
	    cp = globone(dp = Dfix1(cp), G_IGNORE);
	    (void) strncpy(tmp, short2str(cp), MAXPATHLEN);
	    tmp[MAXPATHLEN] = '\0';
	    xfree((ptr_t) cp);
	    xfree((ptr_t) dp);
	    if ((fd = open(tmp, O_RDONLY)) < 0)
		stderror(ERR_SYSTEM, tmp, strerror(errno));
	    (void) dmove(fd, 0);
	}
	else if (flags & F_PIPEIN) {
	    (void) close(0);
	    (void) dup(pipein[0]);
	    (void) close(pipein[0]);
	    (void) close(pipein[1]);
	}
	else if ((flags & F_NOINTERRUPT) && tpgrp == -1) {
	    (void) close(0);
	    (void) open(_PATH_DEVNULL, O_RDONLY);
	}
	else {
	    (void) close(0);
	    (void) dup(OLDSTD);
#ifdef FIOCLEX
#ifdef CLEX_DUPS
	    /*
	     * PWP: Unlike Bezerkeley 4.3, FIONCLEX for Pyramid is preserved
	     * across dup()s, so we have to UNSET it here or else we get a
	     * command with NO stdin, stdout, or stderr at all (a bad thing
	     * indeed)
	     */

	    (void) ioctl(0, FIONCLEX, NULL);
#endif				/* CLEX_DUPS */
#endif				/* FIONCLEX */
	}
    }
    if (cp = t->t_drit) {
	char    tmp[MAXPATHLEN+1];

	cp = globone(dp = Dfix1(cp), G_IGNORE);
	(void) strncpy(tmp, short2str(cp), MAXPATHLEN);
	tmp[MAXPATHLEN] = '\0';
	xfree((ptr_t) dp);
	xfree((ptr_t) cp);
	/*
	 * so > /dev/std{out,err} work
	 */
	(void) dcopy(SHOUT, 1);
	(void) dcopy(SHDIAG, 2);
	if ((flags & F_APPEND) &&
#ifdef O_APPEND
	    (fd = open(tmp, O_WRONLY | O_APPEND)) >= 0);
#else
	    (fd = open(tmp, O_WRONLY)) >= 0)
	    (void) lseek(1, (off_t) 0, L_XTND);
#endif
	else {
	    if (!(flags & F_OVERWRITE) && adrof(STRnoclobber)) {
		if (flags & F_APPEND)
		    stderror(ERR_SYSTEM, tmp, strerror(errno));
		chkclob(tmp);
	    }
	    if ((fd = creat(tmp, 0666)) < 0)
		stderror(ERR_SYSTEM, tmp, strerror(errno));
	}
	(void) dmove(fd, 1);
	is1atty = isatty(1);
    }
    else if (flags & F_PIPEOUT) {
	(void) close(1);
	(void) dup(pipeout[1]);
	is1atty = 0;
    }
    else {
	(void) close(1);
	(void) dup(SHOUT);
	is1atty = isoutatty;
#ifdef FIOCLEX
#ifdef CLEX_DUPS
	(void) ioctl(1, FIONCLEX, NULL);
#endif				/* CLEX_DUPS */
#endif				/* FIONCLEX */
    }

    (void) close(2);
    if (flags & F_STDERR) {
	(void) dup(1);
	is2atty = is1atty;
    }
    else {
	(void) dup(SHDIAG);
	is2atty = isdiagatty;
#ifdef FIOCLEX
#ifdef CLEX_DUPS
	(void) ioctl(2, FIONCLEX, NULL);
#endif				/* CLEX_DUPS */
#endif				/* FIONCLEX */
    }
    didfds = 1;
}

void
mypipe(pv)
    register int *pv;
{

    if (pipe(pv) < 0)
	goto oops;
    pv[0] = dmove(pv[0], -1);
    pv[1] = dmove(pv[1], -1);
    if (pv[0] >= 0 && pv[1] >= 0)
	return;
oops:
    stderror(ERR_PIPE);
}

static void
chkclob(cp)
    register char *cp;
{
    struct stat stb;

    if (stat(cp, &stb) < 0)
	return;
    if ((stb.st_mode & S_IFMT) == S_IFCHR)
	return;
    stderror(ERR_EXISTS, cp);
}
