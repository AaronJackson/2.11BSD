#include	"parms.h"
#include 	"structs.h"

#ifdef	RCSIDENT
static char rcsid[] = "$Header: miscio.c,v 1.7.0.4 96/3/21 23:06:01 notes Rel $";
#endif	RCSIDENT


/*  miscio stuff:
 *
 * ttystrt/ttystop switch back and forth to character-at-a-time mode
 *
 * catchem makes our program ignore kills and coredumps.
 *
 * getnum is a char at a time input routine
 *
 * gchar sucks in 1 character. masks off parity. Uses raw mode to do this
 *
 */


#include <signal.h>
#include <errno.h>


#ifdef	USG						/* Bell's Sys III and V */
#include <termio.h>
struct termio   tty,
                otty;
#endif	defined(USG)

#if	defined(V7) || defined(BSD4x) || defined(BSD2x)
							/* Standard Bell V7 and Berkeley too */
#include <sgtty.h>
int     oldmode;					/* prev mode bits */
int     oldlocalbits;					/* prev local bits */
struct sgttyb   tty;
#endif

char    ttyerase,					/* erase 1 char */
        ttykill;					/* erase entire line */
int     echoctl = 0;					/* as ^G or bell */
int     modeset = 0;					/* == 1 if ttyflags messed with */
short   ospeed;						/* for tputs padding */



ttystrt ()
{
#if	defined(BSD2x) || defined(BSD4x)
    int     localbits;
#endif

    echoctl = 0;					/* default to non-echo */
/*
 *	Grab the current tty state
 */

#ifdef	V7						/* V7 has simple tty controls */
    if (gtty (0, &tty) < 0)				/* failure to grab */
    {
	fprintf (stderr, "%s: Unable to gtty\n", Invokedas);
	exit (1);
    }
#endif	defined(V7)

#ifdef	USG						/* BTL SYS III & V  */
    if (ioctl (0, TCGETA, &tty) < 0 ||
	    ioctl (0, TCGETA, &otty) < 0)		/* one failed */
    {
	fprintf (stderr, "%s: Unable to get tty state\n", Invokedas);
	exit (1);
    }
#endif	defined(USG)

#if	defined(BSD4x) || defined(BSD2x)
							/* Berkeley Unices */
    if (ioctl (0, TIOCGETP, &tty) < 0 ||
	    ioctl (0, TIOCLGET, &oldlocalbits) < 0)
    {
	fprintf (stderr, "%s: Unable to get tty states\n");
	exit (1);
    }
    if (oldlocalbits & LCTLECH)
	echoctl = 1;
#endif

/*
 *	Modify the state to what we want:  cbreak, fix tildes for cursor
 */

#if	defined(USG)
    tty.c_lflag &= NOT ICANON;
    tty.c_cc[VEOF] = 1;
    tty.c_cc[VEOL] = 1;
    ospeed = tty.c_cflag & CBAUD;
    ttyerase = tty.c_cc[VERASE];			/* grab erase char */
    ttykill = tty.c_cc[VKILL];				/* and kill char */
#endif	defined(USG)

#if	defined(BSD4x) || defined(BSD2x) || defined(V7)
    oldmode = tty.sg_flags;
    tty.sg_flags |= CBREAK;
    ospeed = tty.sg_ospeed;				/* speed of terminal */
    ttyerase = tty.sg_erase;				/* grab erase character */
    ttykill = tty.sg_kill;				/* and kill character */
#endif


/*
 *	Now actually tell the system that we want it this way
 */

#if	defined(V7)
    if (stty (0, &tty) < 0)				/* failed */
    {
	fprintf (stderr, "%s: Unable to stty\n", Invokedas);
	exit (1);
    }
#endif	defined(V7)

#if	defined(USG)
    if (ioctl (0, TCSETA, &tty) < 0)
    {
	fprintf (stderr, "%s: Unable to set tty state\n", Invokedas);
	exit (1);
    }
#endif	defined(USG)

    modeset = 1;
    cmstart ();						/* so can cursor address reliably */
}

ttystop ()
{
    if (modeset)
    {
#if	defined(V7) || defined(BSD4x) || defined(BSD2x)
	tty.sg_flags = oldmode;
#endif

#if	defined(V7)
	if (stty (0, &tty) < 0)				/* vanilla Version 7 */
	    printf ("ttystop: stty");			/* cant use x cause he calls us */
#endif	defined(V7)

#if	defined(USG)
	if (ioctl (0, TCSETA, &otty) < 0)		/* Unix 4.0 */
	    printf ("ttystop: stty");			/* cant use x cause he calls us */
#endif	defined(USG)

#if	defined(BSD4x) || defined(BSD2x)
	if ((ioctl (0, TIOCSETN, &tty) < 0) ||
		(ioctl (0, TIOCLSET, &oldlocalbits) < 0))
	    printf ("ttystop: stty");			/* cant use x cause he calls us */
#endif
    }
    cmstop ();						/* get out of cursor addressing mode */
    modeset = 0;
}


static int  (*osigint) (),
            (*osigquit) ();				/* hold signal status */
#if	 defined(SIGTSTP)
static int  (*osigtstp) ();				/* control-z job stop */
#endif	defined(SIGTSTP)

catchint ()
{
    intflag = 1;
    signal (SIGINT, catchint);				/* fix em up again */
#ifndef	DEBUG
    signal (SIGQUIT, catchint);
#endif	DEBUG
}

#if	defined(SIGTSTP)
catchz ()						/* handle ^Z gracefully */
{
    int     wasset;					/* tty mode flag */

    if (ignoresigs)					/* if in critical section */
    {
	signal (SIGTSTP, catchz);			/* re-catch */
	return;						/* and ignore */
    }
    if ((wasset = modeset) != 0)			/* want assignment */
    {
	at (0, 1);					/* go to bottom corner */
	fflush (stdout);
	ttystop ();					/* fix tty modes */
    }

    signal (SIGTSTP, SIG_DFL);				/* make sure it nabs us */
#if	defined(BSD42)
/*
 *	since 4.2 Bsd blocks signals while we are handling them, we
 *	have to explicitly tell the kernel that we want the signals
 *	to come through.
 *	It would probably be more correct to only let some signals
 *	through instead of all.
 */
    (void) sigsetmask(0L);				/* pass signals */
#endif	BSD42
    kill (0, SIGTSTP);					/* halt myself */
    signal (SIGTSTP, catchz);				/* ready to catch again */
    if (wasset)
	ttystrt ();					/* fix his tty */
}
#endif	defined(SIGTSTP)

catchem ()
{
    osigint = signal (SIGINT, catchint);		/* interrupts */
#ifndef	DEBUG
    osigquit = signal (SIGQUIT, catchint);		/* quits */
#endif	DEBUG
#if	defined(SIGTSTP)
    osigtstp = signal (SIGTSTP, catchz);		/* control Z */
#endif
}

uncatchem ()						/* restore signal status */
{
    signal (SIGINT, osigint);
#ifndef	DEBUG
    signal (SIGQUIT, osigquit);
#endif	DEBUG
#if	defined(SIGTSTP)
    signal (SIGTSTP, osigtstp);
#endif
}

gchar ()
/*
 *	Return next character from tty.
 *	this is all done in cbreak mode of course
 */
{
    char    c;
    register int    retcode;
    fflush (stdout);					/* get rid of what's there */
    while ((retcode = read (0, &c, 1)) <= 0)		/* try reading */
	if (retcode == 0 || errno != EINTR)		/* if bizarre */
	{
	    fprintf (stderr, "%s: Bad tty read\n", Invokedas);
	    exit (1);
	}
    intflag = 0;					/* remove any pending */

    return (c & 0177);
}

/*
 *	getnum (c) 
 *	grab a number from the terminal. "c" is the first digit o 
 *	the number. 
 *	
 *	Originally coded:	Rob Kolstad	Fall 1980
 *	Modified:		Ray Essick (with help from Malcolm Slaney)
 *						July 1982
 *				to handle user defined erase and kill
 *				characters.
 */

getnum (c)						/* c is the initial character! */
char    c;
{
    int     num,
            numin;
    int     i,
            j;

    num = c - '0';
    numin = 1;
    putc (c, stdout);
    while (1)
    {
	c = gchar ();					/* get next digit */
	if (c == ttyerase)				/* want to erase? */
	{
	    if (numin > 0)				/* if have some */
	    {
		switch (c)				/* zap the char */
		{
		    case '\b': 				/* physical backspace */
			printf (echoctl ? "\b\b  \b\b" : " \b");
			break;
		    default: 
			for (i = 0, j = widthch (c); i < j; i++)
			    printf ("\b \b");
			break;
		}
		printf ("\b \b");			/* and the digit */
		numin--;
		num /= 10;				/* correct num */
	    }
	    else
	    {						/* nothing to zap */
		switch (c)				/* cover the erase */
		{
		    case '\b': 
			printf (echoctl ? "\b\b  \b\b" : " ");
			break;
		    default: 
			for (i = 0, j = widthch (c); i < j; i++)
			    printf ("\b \b");		/* erase it */
			break;
		}
	    }
	}
	else
	    if (c == ttykill)
	    {
		num = 0;				/* blast it away */
		for (i = 0, j = widthch (c); i < j; i++)
		    printf ("\b \b");
		while (numin > 0)			/* erase the screen */
		{
		    numin--;
		    printf ("\b \b");
		}
		numin = 0;				/* in case */
	    }
	    else
		switch (c)
		{
		    case '\n': 
		    case '\r': 
			return num;			/* done */

		    default: 
			if (c < '0' || c > '9')
			{
			    printf ("\10 \10\07");
			    continue;
			}
			numin++;
			num = 10 * num + (c - '0');
			break;
		}
    }
}

/*
 *	gline( p, i) - suck a maximum of i characters from the tty.
 *	do erase and kill processing.
 *	The line is terminated by the user typing a <cr> or <nl>. This
 *	character is converted to null and left on the end of the
 *	string returned. The count of characters (including the null
 *	terminator) is returned.
 *	The array passed in is assumed to have i+1 elements
 *	(enough for the characters plus the terminator)
 *
 *	Original Coding:	Ray Essick	December 1981
 *	Repaired to use user's erase and kill characters
 *				Malcolm Slaney	July 1982
 *
 */

gline (p, max)
char   *p;
{
    register int    numin;
    register char  *q;					/* pointer to buffer */
    register char   c;					/* hold the input character */
    register int    i,
                    j;

    q = p;						/* get base */
    numin = 0;
    while (1)
    {
	c = gchar ();					/* & flushes stdout */
	if (c == ttyerase)				/* want to erase */
	{
	    if (numin > 0)
	    {
		numin--;
		q--;					/* back up in buffer */
		switch (c)
		{
		    case '\b': 				/* physical backspace */
			printf (echoctl ? "\b\b\b   \b\b\b" : " \b");
			break;
		    default: 				/* anything else */
			j = widthch (c) + widthch (*q);
			for (i = 0; i < j; i++)
			    printf ("\b \b");
			break;
		}
	    }
	    else
	    {
		switch (c)
		{
		    case '\b': 				/* really backspaces */
			printf (echoctl ? "\b\b  \b\b" : " ");
			break;
		    default: 
			j = widthch (c);
			for (i = 0; i < j; i++)
			    printf ("\b \b");
			break;
		}
	    }
	}
	else
	    if (c == ttykill)
	    {
		for (i = 0; i < widthch (c); i++)
		    printf ("\b \b");
		for (i = 0; i < numin; i++)		/* for all our chars */
		{
		    --q;				/* get to char */
		    for (j = 0; j < widthch (*q); j++)	/* zap it */
			printf ("\b \b");
		}
		q = p;					/* reset pointer */
		numin = 0;				/* in case .. */
	    }
	    else
		switch (c)
		{
		    case '\n': 
		    case '\r': 
			if (numin >= max)		/* should only ever be = */
			{
			    p[max] = '\0';		/* put a null at the end */
			    return max + 1;		/* which is how many we return */
			}
			*q = '\0';
			numin++;
			return numin;

		    case '\\': 				/* escape character */
			printf ("\010");		/* back space to it */
			c = gchar ();			/* grab escaped character */
							/* and fall through to default */

		    default: 				/* add character to buffer */
			if (numin < max)
			{
			    *q++ = c;
			    numin++;
			}
			else
			{
			    printf ("\10 \10");		/* show him I ignored char */
			}
			break;
		}
    }
}

askyn (p) char *p;					/* returns y or n to the question */
{
    char    c;						/* return temp */
    printf ("%s", p);
    while (1)
    {
	c = gchar ();
	if (c == 'y' || c == 'n')
	    break;
	printf ("\07 y or n please\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
    }
    return c;
}

/*
 * return 1 if there is input from the terminal,
 * 0 otherwise.  systems without the appropriate
 * call should always return 0.
 */
isinput ()
{
#ifdef	FIONREAD					/* BSD 4.1, 4.1a, 4.2 */
    long    retval;
    if (ioctl (0, FIONREAD, &retval))
	return 0;					/* failed, say no input */
    return (retval != 0);				/* count of characters */
#endif	FIONREAD

    return 0;						/* for other systems */
}

/*
 *	mapch(c) char c;
 *
 *	prints control characters as ^x style. 
 *	others as normal.
 */
mapch (c)
char    c;
{
    if (c < 040)
    {
	putchar ('^');
	putchar (c | 0100);				/* make visible */
    }
    else
	if (c == 0177)
	{
	    putchar ('^');
	    putchar ('?');
	}
	else
	    putchar (c);
}

/*
 *	widthch(c)
 *
 *	returns the number of character positions this character uses
 *	when displayed.
 */

int     widthch (c)
char    c;
{
    if (c == '\b')
	return (echoctl ? 2 : -1);
    if (c < 040 || c == 0177)				/* control character */
	return (2 * echoctl);				/* 0 or 2 */
    return (1);						/* normal character */
}
