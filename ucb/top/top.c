/*
 * Display processes in order of CPU utilization
 *
 * Implemented for 2.11 BSD to evaluate mechanisems for accessing
 * system system state from user mode.
 * Process information access is modeled on mechanism used by ps(1)
 * in particular, makes use of the cached static information in
 *  /var/run/psdatabase
 * which is generated for the ps program at system boot
 *
 * Digby Tarvin 
 *  December 2019.
 *
 * History
 * =======
 * Ed Date       What                                             Who
 * -- ---------- -----------------------------------------------  ----
 * 01 05/12/2019 Written                                          DRST
 * 02 07/03/2020 Removed kmem based nswap access (sysctl updated) DRST
 * 03 09/03/2020 added BADKEY_IGNORE, delay spec now in seconds   DRST
 * 04 12/03/2020 Changed to use curses, correct memory size info  bqt
 * 05 21/03/2020 Corrected memory calculations to be unsigned     bqt
 * 06 29/08/2021 Changed and corrected uptime calculation.        bqt
 *               Changed header layout.
 *               Added %cpu time to header.
 * 07 26/8/2022  Truncate hostname by first period or 7 char.     bqt
 */
/*
 * Using RAW removes the need for signal processing, but adds a requirement
 * to process NL -> CRNL mapping etc. Choose your preference...
 */
#define TTY_RAW    	0	/* use RAW/!CBRK for terminal */
#define BADKEY_IGNORE	1	/* no abort on invalid commands */
#define TIME_CHILD	0	/* include time of children in process */

#include <stdio.h>
#include <unistd.h>	/* usleep */
#include <stdlib.h>	/* getopt */
#include <string.h>
#include <sys/param.h>	/* NGROUPS */
#include <sys/sysctl.h>	/* boottime,.. */
#include <ctype.h>	/* isdigit */
#include <sys/file.h>	/* lseek */
#include <sys/stat.h>	/* struct stat */
#include <sys/user.h>	/* MAXCOMLEN */
#include <sys/proc.h>	/* struct proc */
#include <sys/types.h>
#include <sys/time.h>	/* select() */
#include <sys/select.h>	/* select() */
#include <sys/signal.h>	/* select() */
#include <sys/ioctl.h>	/* winsize, TIOCGETP */
#include <ctype.h>	/* isdigit() */
#include <pwd.h>
#include <utmp.h>	/* PS_NAMESIZE, _PATH_UTMP */
#include <curses.h>
#include <sys/dk.h>	/* cpu stats */
#include <nlist.h>
#include "psdb.h"

#define LOCAL	static
#define EXPORT	/**/

typedef struct udata_s
{
	dev_t	o_ttyd;		/* u_ttyd */
	char	o_tty[3];	/* 1st 2 chars of tty after 'tty' */
	char	*o_uname;	/* login name of process owner */
	time_t	o_utime;	/* u_utime */
	time_t	o_stime;	/* u_stime */
#if TIME_CHILD
	time_t	o_cutime;	/* u_cutime */
	time_t	o_cstime;	/* u_cstime */
#endif
	time_t	o_ttime;	/* u_utime + u_stime */
	time_t	o_dtime;	/* o_ttime - o_LastTime */
	int	o_sigs;		/* sum of SIGINT & SIGQUIT (2 => ignore both) */
	size_t	o_tsize;	/* text size */
	int	o_increment;	/* current - last */
	char	o_comm[MAXCOMLEN];	/* u_comm */
	char	*o_args;	/* best guess at args to process */
	long	o_nswap;	/* swaps */
	long	o_nvcsw;	/* voluntary context switches */
	long	o_nicsw;	/* involuntary " */
	/*
	 * details from last update
	 */
	short	o_LastPid;	/* last pid in this slot */
	short	o_LastStat;	/* is previous process data valid? */
	time_t	o_LastTime;	/* time at last update */
} udata_t;

#define DRATE	(1000000)	/* default delay between updates (us) */
#define MAXUSER	10		/* user names to cache */

LOCAL struct userdb_s
{
	char	uname[UT_NAMESIZE];
	short	uid;
} untab[MAXUSER];

struct nlist nl[] = {
#define X_CPTIME 0
	{ "_cp_time" },
	{ "" },
};

LOCAL long		upd_us = DRATE;	/* delay between updates */
LOCAL struct proc	*proctab;	/* buffer to contain proc array */
LOCAL int		ptsz = 0;	/* size of proc array */
LOCAL int		*procidx;	/* indirect array for proc access */
LOCAL struct udata_s	*userdata;	/* process information from u */

LOCAL char		xflg;
LOCAL char		gflg;
LOCAL char		*tptr;
LOCAL char		*memf	= "/dev/mem";
LOCAL char		*kmemf	= "/dev/kmem";
LOCAL char		*swapf	= "/dev/swap";

LOCAL int		sort = 9;
LOCAL int		idle = 1;

LOCAL int		kmem;
LOCAL int		mem;
LOCAL int		swap;
LOCAL u_int		nproc;
LOCAL int		hz;
LOCAL float		fhz;
LOCAL int		done = 0;
LOCAL struct tchars	tc;	/* determine t_intrc and t_quitc */

/* system stats */
LOCAL time_t		now;
LOCAL struct tm		*ltp;
LOCAL struct timeval	boottime;
LOCAL time_t		uptime;
LOCAL FILE		*ut;
LOCAL struct utmp	utmp;
LOCAL int		nusers;
LOCAL double		avenrun[3];
LOCAL char		host[8];

LOCAL struct  map_s
{
        off_t   b1, e1;
	off_t   f1;
        off_t   b2, e2;
	off_t   f2;
} datmap;

/*
 * Configure/restore output terminal
 */
LOCAL int config_tty(onoff)
int	onoff;
{
	if(onoff == 0) {
		resetty();
	} else {
		initscr();
		savetty();
		raw();
		noecho();
	}
	return(0);
}

/*
 * Read from specified location in file
 *  returns 1 on success
 *         -1 on failure
 */
LOCAL int readat(fd, pos, buff, count)
int 		fd;
off_t		pos;
char 		*buff;
unsigned short	count;
{
	if(lseek(fd, pos, L_SET) == (off_t) -1)
		return(-1);
	return((read(fd, buff, count) == count)? 1 : -1);
}

/*
 ******* Get Information for a process given a proc array entry ********
 */
#define	within(x,y,z)	(((unsigned)(x) >= (y)) && ((unsigned)(x) < (z)))

LOCAL getbyte(adr, file)
register char	*adr;
int		file;
{
	register struct	map_s *amap = &datmap;
	char	b;
	off_t	saddr;

	if(!within(adr, amap->b1, amap->e1))
	{
		if(within(adr, amap->b2, amap->e2))
			saddr = (unsigned) adr + amap->f2 - amap->b2;
		else
			return(0);
	}
	else
	{
		saddr	= (unsigned) adr + amap->f1 - amap->b1;
	}
	if(lseek(file, saddr, L_SET) == (off_t) -1 || read (file, &b, 1) < 1)
		return(0);
	return((unsigned) b);
}

LOCAL char	*
getptr(adr, file)
char	**adr;
int	file;
{
	char	*ptr = 0;
	register char	*p, *pa;
	register int	i;

	pa = (char *)adr;
	p  = (char *)&ptr;
	for (i = 0; i < sizeof (ptr); i++)
		*p++ = getbyte(pa++, file);
	return(ptr);
}
/*
 * Get process command line
 * Passed:
 *	fd   - descriptor for process image (mem or swap as appropriate)
 *	stk  - offset to process stack area (mem or swap in file)
 *	pp   - proc structure for process
 *	a    - pointer to process summary structure to update
 *
 * Returns:
 *	1    - success
 *      0    - failed
 */
/* amount of top of stack to examine for args */
#define ARGLIST	(1024/sizeof(int))

LOCAL getargs(fd, stk, pp, a, u)
int			fd;
off_t			stk;
struct proc		*pp;
register struct udata_s	*a;
struct user		*u;
{
	register int	*ip;
	register char	*cp, *cp1;
	char		c, **ap;
	int		cc, nbad, abuf[ARGLIST];
	off_t		pos;

	if(a->o_args == NULL)
	{
		if((a->o_args = (char *) malloc(64)) == NULL)
			return(1);
	}
	a->o_args[0] = 0;
	stk += ctob((off_t) pp->p_ssize) - ARGLIST*sizeof(int);

	/* look for sh special */
	pos = stk + ARGLIST*sizeof(int) - sizeof (char **);
	if(readat(fd, pos, (char *)&ap, sizeof(char *)) < 0)
		return(1);

	if(ap)
	{
		char	b[82];
		char	*bp	= b;
		while((cp = getptr(ap++, fd)) && cp && (bp < b+sizeof (a->o_args)) )
		{
			nbad	= 0;
			while( (c = getbyte(cp++, fd))&&(bp < b+sizeof (a->o_args)))
			{
				if(c<' ' || c > '~')
				{
					if(nbad++ > 3)
						break;
					continue;
				}
				*bp++ = c;
			}
			*bp++ = ' ';
		}
		*bp++ = 0;
		(void)strcpy(a->o_args, b);
		return(1);
	}

	if(readat(fd, stk, (char *) abuf, sizeof (abuf)) < 0)
		return(1);

	abuf[ARGLIST-1]	= 0;
	for(ip = &abuf[ARGLIST-2]; ip > abuf;)
	{
		if(*--ip == -1 || *ip == 0)
		{
			cp = (char *) (ip + 1);
			if(*cp == '\0')
				cp++;
			nbad = 0;
			for (cp1 = cp; cp1 < (char *) &abuf[ARGLIST]; cp1++)
			{
				cc = *cp1 & 0177;
				if(cc == 0)
					*cp1 = ' ';
				else if(cc < ' ' || cc > 0176)
				{
					if(++nbad >= 5)
					{
						*cp1++	= ' ';
						break;
					}
					*cp1 = '?';
				}
				else if(cc == '=')
				{
					*cp1 = '\0';
					while(cp1 > cp && *--cp1 != ' ')
						*cp1 = '\0';
					break;
				}
			}
			while(*--cp1 == ' ')
				*cp1 = 0;

			(void)strcpy(a->o_args, cp);
garbage:
			cp = a->o_args;
			if(cp[0] == '-'&&cp[1]<=' '||cp[0]=='?'||cp[0]<=' ')
			{
				strcat(cp, " (");
				strcat(cp, u->u_comm);
				strcat(cp, ")");
			}
			cp[63] = 0;	/* max room in udata_s is 64 chars */
			if(xflg || gflg || tptr || cp[0] != '-')
				return(1);
			return(0);
		}
	}
	goto garbage;
}

LOCAL char *
gettty(ttyp, ttyd)
struct tty	*ttyp;
dev_t		ttyd;
{
	register int tty_step;
	register char *p = "?";

	if(ttyp == NULL)
		return(p);

	for(tty_step = 0;tty_step < nttys; ++tty_step)
		if(ttlist[tty_step].ttyd == ttyd)
		{
			p = ttlist[tty_step].name;
			if(!strncmp(p,"tty",3))
				p += 3;
		}

	return(p);
}
/*
 * Save process information to udata_s structure
 */

LOCAL int fuser(idx, uid)
int	idx;
short	uid;
{
	register struct passwd *pw;

	if((pw = getpwuid(uid)) == NULL)
		return(-1);
	untab[idx].uid = uid;
	strcpy(untab[idx].uname, pw->pw_name);
	return(1);
}

#define	round(x,y) ((long) ((((long) (x) + (long) (y) - 1L) / (long) (y)) * (long) (y)))

LOCAL getu(pp, a, noargs)
struct proc	*pp;
struct udata_s	*a;
char		noargs;
{
	char			*tp;
	off_t			addr;
	off_t			daddr;
	off_t			saddr;
	struct user		user;		/* for reading user structs */
	register struct	user	*up	= &user;
	long			txtsiz, datsiz, stksiz;
	int			septxt;
	int			pifile;
	int			i;

	if(pp->p_flag & SLOAD)	/* in memory */
	{
		addr = ctob((off_t) pp->p_addr);
		daddr = ctob((off_t) pp->p_daddr);
		saddr = ctob((off_t) pp->p_saddr);
		pifile = mem;
	}
	else				/* swapped */
	{
		addr = (off_t)pp->p_addr << 9;
		daddr = (off_t)pp->p_daddr << 9;
		saddr = (off_t)pp->p_saddr << 9;
		pifile = swap;
	}

	if(readat(pifile, addr, (char *) up, sizeof(user)) < 0)
	{
		perror("read");
		return(0);
	}

	txtsiz		= ctob(up->u_tsize);	/* address maps for usr pcs */
	datsiz		= ctob(up->u_dsize);
	stksiz		= ctob(up->u_ssize);
	septxt		= up->u_sep;
	datmap.b1	= (septxt ?  0 : round(txtsiz, TXTRNDSIZ));
	datmap.e1	= datmap.b1 + datsiz;
	datmap.f1	= daddr;
	datmap.f2	= saddr;
	datmap.b2	= stackbas(stksiz);
	datmap.e2	= stacktop(stksiz);
	tp		= gettty(up->u_ttyp, up->u_ttyd);
	strncpy(a->o_tty, tp, sizeof (a->o_tty)-1);
	a->o_tty[sizeof(a->o_tty)-1] = 0;
	a->o_ttyd	= tp[0] == '?' ?  -1 : up->u_ttyd;

	a->o_tsize	= 0;
	a->o_utime	= 0;
	a->o_stime	= 0;
	a->o_nswap	= 0;
	a->o_nvcsw	= 0;
	a->o_nicsw	= 0;
#if TIME_CHILD
	a->o_cutime	= 0;
	a->o_cstime	= 0;
#endif
	a->o_ttime	= 0;
	a->o_sigs	= 0;
	a->o_dtime	= 0;
	a->o_uname	= NULL;
	a->o_comm[0]	= 0;

	if(pp->p_stat == SZOMB)
		return(1);

	a->o_tsize	= up->u_tsize;
	a->o_utime	= up->u_ru.ru_utime;
	a->o_stime	= up->u_ru.ru_stime;
	a->o_nswap	= up->u_ru.ru_nswap;
	a->o_nvcsw	= up->u_ru.ru_nvcsw;
	a->o_nicsw	= up->u_ru.ru_nivcsw;
#if TIME_CHILD
	a->o_cutime	= up->u_cru.ru_utime;
	a->o_cstime	= up->u_cru.ru_stime;
	a->o_ttime	= a->o_utime + a->o_stime + a->o_cutime + a->o_cstime;
#else
	a->o_ttime	= a->o_utime + a->o_stime;
#endif
	a->o_sigs	= (int)up->u_signal[SIGINT]+(int)up->u_signal[SIGQUIT];
	if(a->o_LastStat && (pp->p_pid == a->o_LastPid))
		a->o_dtime = a->o_ttime - a->o_LastTime;
	else
		a->o_dtime = a->o_ttime;

	for(i = 0; i < MAXUSER; i++)
	{
		if(untab[i].uname[0] == 0)
			break;
		if(untab[i].uid == pp->p_uid)
		{
			a->o_uname = untab[i].uname;
			break;
		}
	}
	if((a->o_uname == NULL) && (i< MAXUSER))
	{
		/* name not found and space in table */
		if(fuser(i, pp->p_uid) >= 0)
			a->o_uname = untab[i].uname;	
	}

	strncpy(a->o_comm, up->u_comm, MAXCOMLEN-1);
	a->o_comm[MAXCOMLEN-1] = 0;

	if(noargs)
		return(1);
	return(getargs(pifile, saddr, pp, a, up));
}
/*
 * Compare function for process sorting
 */
LOCAL int sortcmp(i1, i2)
register int *i1, *i2;
{
	struct proc *p1, *p2;
	struct udata_s *u1, *u2;

	p1 = &proctab[*i1];
	p2 = &proctab[*i2];
	u1 = &userdata[*i1];
	u2 = &userdata[*i2];

	switch (sort) {
	case 0:
		return p1->p_pid - p2->p_pid;
	case 1:
		return strcmp(u1->o_uname, u2->o_uname);
	case 2:
		return p2->p_pri - p1->p_pri;
	case 3:
		return p1->p_nice - p2->p_nice;
	case 4:
		return u2->o_tsize - u1->o_tsize;
	case 5:
		return p2->p_dsize - p1->p_dsize;
	case 6:
		return p2->p_ssize - p1->p_ssize;
	case 7:
		return p1->p_stat - p2->p_stat;
	case 9:
		{
			time_t t;
			t = u2->o_dtime - u1->o_dtime;
			if (t) return t;
		}
	case 8:
		return u2->o_ttime - u1->o_ttime;
	case 10:
		return strcmp(u1->o_comm, u2->o_comm);
	}
	return 0;
}
/*
 * Update the process list
 */
LOCAL int update()
{
	int	np;
	int	i;

	/*
	 * before over-writing old proc structure,
	 * 	record old stat and if applicable, pid and ttime 
	 */
	for(i = 0; i < nproc; i++)
	{
		struct proc 		*procp	= &proctab[i];
		register struct	udata_s	*up	= &userdata[i];

		if((up->o_LastStat = procp->p_stat) != 0)
		{
			up->o_LastTime	= up->o_ttime;
			up->o_LastPid	= procp->p_pid;
		}
	}

	/* read the current proc table into buffer */
	if(readat(kmem, (off_t) proc_sym, proctab, ptsz) < 0)
	{
		fprintf(stderr, "Error reading process table\r\n");
		return(-1);
	}

	for(np = i = 0; i < nproc; i++)
	{
		struct proc *procp = &proctab[i];

		if(procp->p_stat == 0)
			continue;

		if(getu(procp, &userdata[i], 1) == 0)
		{
			fprintf(stderr, "Error getting process %d uinf\r\n",
				procp->p_pid);
			return(-1);
		}
		procidx[np] = i;
		np++;
	}
	return(np);
}

LOCAL char states[] = { '?', 'S', 'W', 'R', 'I', 'Z', 'T' };

/*
 * Get Memory Statistics for summary lines
 * Returns:
 *   totmem  - total physical memory in bytes
 *   freemem - available memory in bytes
 */
LOCAL int GetMemStats(totmem, freemem)
unsigned long *totmem;
unsigned long *freemem;
{
	int		mib[2];
	size_t		size;
	static size_t	cmsz	= 0;
	static unsigned long 	physmem	= 0;
	static char	*coremap = NULL;
	struct mapent	*map;
	int		i;
	unsigned long	tfree;

	if(physmem == 0)
	{
		/* first call - get static values */
		mib[0] = CTL_HW;
		mib[1] = HW_PHYSMEM;
		size = sizeof(physmem);
		if(sysctl(mib, 2, &physmem, &size, NULL, 0) == -1)
		{
			printw("PHYSMEM: %s\n\n", strerror(errno));
			return(-1);
		}
		mib[0] = CTL_VM;
		mib[1] = VM_COREMAP;
		if(sysctl(mib, 2, NULL, &cmsz, NULL, 0) == -1)
		{
			printw("COREMAP: %s\n\n", strerror(errno));
			return(-1);
		}
		if((coremap = (char *)malloc((unsigned)cmsz)) == NULL)
		{
			perror("COREMAP");
			printf("\r\n");
			exit(1);
		}
		memset(coremap, 0, cmsz);
	}
	mib[0] = CTL_VM;
	mib[1] = VM_COREMAP;
	size = cmsz;
	if(sysctl(mib, 2, coremap, &size, NULL, 0) == -1)
	{
		printw("COREMAP2: %s\n\n", strerror(errno));
		return(-1);
	}
	
	map = (struct mapent *)coremap;
	for(tfree = i = 0; i < cmsz / sizeof(struct mapent); i++)
		tfree += map[i].m_size;
	*totmem = physmem;
	*freemem = (long) tfree*64;
	return(0);
}

/*
 * Get Swap Statistics for summary lines
 * Returns:
 *   totmem  - total physical memory in bytes
 *   freemem - available memory in bytes
 */

LOCAL int GetSwapStats(totswap, freeswap)
unsigned long *totswap;
unsigned long *freeswap;
{
	int		mib[2];
	size_t		size;
	static size_t	smsz	= 0;
	static unsigned long 	nswap	= 0;
	static char	*swapmap = NULL;
	struct mapent	*map;
	unsigned int	i;
	unsigned long	tfree;

	if(nswap == 0)
	{
		/* first call - get static values */
		mib[0] = CTL_VM;
		mib[1] = VM_NSWAP;
		size = sizeof(i);
		if(sysctl(mib, 2, &i, &size, NULL, 0) == -1)
		{
			printw("NSWAP: %s\n\n", strerror(errno));
			return(-1);
		}
		nswap = i;
		mib[0] = CTL_VM;
		mib[1] = VM_SWAPMAP;
		if(sysctl(mib, 2, NULL, &smsz, NULL, 0) == -1)
		{
			printw("SWAPMAP: %s\n\n", strerror(errno));
			return(-1);
		}
		if((swapmap = (char *)malloc((unsigned)smsz)) == NULL)
		{
			perror("SWAPMAP");
			printf("\r\n");
			exit(1);
		}
		memset(swapmap, 0, smsz);
	}
	mib[0] = CTL_VM;
	mib[1] = VM_SWAPMAP;
	size = smsz;
	if(sysctl(mib, 2, swapmap, &size, NULL, 0) == -1)
	{
		printw("SWAPMAP2: %s\n\n", strerror(errno));
		return(-1);
	}
	
	map = (struct mapent *)swapmap;
	for(tfree = i = 0; i < smsz / sizeof(struct mapent); i++)
		tfree += map[i].m_size;
	*totswap = nswap*512;
	*freeswap = (unsigned long) tfree*512;
	return(0);
}

/*
 * Show summary stats
 */
LOCAL void
show_stats(npr)
int npr;
{
	int	i, run, sleep, stop, zombie;
	static int calls = 0;
	int	mib[2];
	size_t	size;
	unsigned long	memsize, memfree;
	unsigned long	swapsize, swapfree;
	long s_time[CPUSTATES], t, etime;
	static long s1_time[CPUSTATES] = {0};

	/* get time */
	time(&now);
	ltp = localtime(&now);
	printw("%s - %02d:%02d:%02d", host, ltp->tm_hour, ltp->tm_min, ltp->tm_sec);

	/* get uptime */

	mib[0] = CTL_KERN;
	mib[1] = KERN_BOOTTIME;
	size = sizeof(boottime);
	if(sysctl(mib, 2, &boottime, &size, NULL, 0) != -1 &&
		boottime.tv_sec != 0)
	{
		int	days, hrs, mins;

		uptime = (now - boottime.tv_sec + 30) / 60;
		mins = uptime % 60;
		uptime /= 60;
		hrs = uptime % 24;
		days = uptime / 24;

		printw(" up %2d day%s", days, days != 1?"s":"");
		printw(", %2d:%02d", hrs, mins);
	}

	/* get user count */

	nusers = 0;
	while(fread(&utmp, sizeof(utmp), 1, ut))
		if(utmp.ut_name[0] != '\0')
			nusers++;
	rewind(ut);
	printw(", %3d user%c", nusers, nusers > 1? 's': '\0');

	/* load average */
	if(getloadavg(avenrun, sizeof(avenrun)/sizeof(avenrun[0])) == -1)
		printw(", no load average information available");
	else
	{
		int	i;
		printw(",  load average:");
		for(i = 0; i < (sizeof(avenrun)/sizeof(avenrun[0])); i++)
		{
			if(i > 0)
				printw(",");
			printw(" %.2f", avenrun[i]);
		}
	}
	printw("\n");
	/* process status summary */
	run = sleep = stop = zombie = 0;
	for(i = 0; i < npr; i++)
	{
		register struct proc	*p = &proctab[procidx[i]];
		switch(p->p_stat)
		{
			case SSLEEP: sleep++;  break;
			case SWAIT:  sleep++;  break;
			case SRUN:   run++;    break;
			case SIDL:   run++;    break;
			case SZOMB:  zombie++; break;
			case SSTOP:  stop++;   break;
/*			case 0:      run++;    break; /* ?? */
			default:
				zombie++; /* shouldn't happen */
		}
	}
	printw("Tasks:%4d total, %3d running,", npr, run);
	printw(" %4d sleeping, %3d stopped, %3d zombie\n",
		sleep, stop, zombie);

	/* Cpu states */
	lseek(kmem, (long)nl[X_CPTIME].n_value, L_SET);
	read(kmem, s_time, sizeof s_time),
	etime = 0;
	for(i=0; i < CPUSTATES; ++i)
	{
		t = s_time[i];
		s_time[i] -= s1_time[i];
		s1_time[i] = t;
		etime += s_time[i];
	}
	if(etime == 0) etime = 1;

	printw("Cpu  : %5.1f%% us, %5.1f%% ni, %5.1f%% sy, %5.1f%% id\n",
		((float)s_time[0] / etime) * 100.,
		((float)s_time[1] / etime) * 100.,
		((float)s_time[2] / etime) * 100.,
		((float)s_time[3] / etime) * 100.);

	/* Memory and Swap status */

	if(GetMemStats(&memsize, &memfree) < 0)
		return;
	if(GetSwapStats(&swapsize, &swapfree) < 0)
		return;
	/* 2.11 BSD has static buffer cache so no need to display this */
	printw("Mem  : %7.1fK total, %7.1fK free, %7.1fK used (%2ld%%)\n",
		memsize/1024.0, memfree/1024.0, (memsize-memfree)/1024.0,
		(memsize-memfree)*100L/memsize);
	printw("Swap : %7.1fK total, %7.1fK free, %7.1fK used (%2ld%%)\n",
		swapsize/1024.0, swapfree/1024.0, (swapsize-swapfree)/1024.0,
		(swapsize-swapfree)*100L/swapsize);
}

/*
 * show heading information
 */
LOCAL void
show_titles()
{
	standout();
	printw("  PID USER      PR  NI  TEXT  DATA STACK S     TIME   DTIME COMMAND \n");
	standend();
}

LOCAL void
show_procs(np, max)
int	np;
int	max;
{
	int	i;

#if 0
	printf(" nproc=%d witharg=%d hz = %d\n", nproc, np, hz);
#endif
	qsort((char *)procidx, np, sizeof(int), sortcmp);
	for(i = 0; i < np; i++)
	{
		register struct proc	*p = &proctab[procidx[i]];
		register struct udata_s	*a = &userdata[procidx[i]];

		if (idle || a->o_dtime || p->p_stat == SRUN)
		{
			printw("%5d ",	p->p_pid);
			if(a->o_uname != NULL)
				printw("%-7s", a->o_uname);
			else
				printw("%7d", p->p_uid);
			printw("%5d",	p->p_pri);
			printw("%4d",	p->p_nice);
			printw("%5.1fK", (ctob(a->o_tsize))/1024.0);
			printw("%5.1fK", (ctob(p->p_dsize))/1024.0);
			printw("%5.1fK", (ctob(p->p_ssize))/1024.0);
			printw(" %c",   (p->p_stat>6)? '?' : states[p->p_stat]);
			printw("%c", 	(p->p_flag & SLOAD)? ' ':'s');

#if 1
			printw("%8.2f ", a->o_ttime/fhz);
			printw("%7.2f ", a->o_dtime/fhz);
#else
			printw("%8ld ", a->o_ttime);
			printw("%7ld ", a->o_dtime);
#endif
#if 0
			printw(" %2d %2d %2d ", a->o_nswap, a->o_nvcsw, a->o_nicsw);
			printw(" %4d %1d ", a->o_LastPid, a->o_LastStat);
#endif
			if((a->o_comm[0] == 0)&&(p->p_pid == 0))
				printw("%-19s",    "[swapper]");
			else
				printw("%-19s",    a->o_comm);
#if 1
			if(i >= (max-1))
				break;
#endif
			printw("%s", "\n");
		}
	}
}

/*
 * Wait for a key press on stdin, signal  or until tout microseconds
 * have elapsed
 * returns the key value, -1 on error, or zero if timeout occured
 */
LOCAL int keypress(usec)
long usec;
{
	struct timeval	tv;
	fd_set		rfds;

	if(done)
		return('q');	/* signal */

	tv.tv_sec	= usec / 1000000;
	tv.tv_usec	= usec % 1000000; 
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);

	if(select(1, &rfds, NULL, NULL, &tv) > 0)
	{
		char c;
		if(read(0, &c, 1) != 1)
			return(-1);
		if((c == tc.t_intrc)||(c == tc.t_quitc))
			c = 'q';
		return(c & 0x7f);
	}
	return(0);
}

/*
 * Convert and return a ms value formatted s[.d[d[d]]]
 * returns delay on success
 *        -1 on invalid delay spec
 */
LOCAL long GetDelay(ds)
char *ds;
{
	char *p;
	char *dp = NULL;
	long delay = 0;

	/*check valid syntax delay spec */
	for(p = ds; *p; p++)
	{
		if(*p == '.')
		{
			if(dp != NULL)
				return(-1);
			dp = p;
			continue;
		}
		if(!isdigit(*p))
			return(-1);
	}
	/* do conversion */
	if(dp != NULL)	/* subseconds specified */
	{
		int	dd;	/* decimal digits */

		*dp++ = 0;
		dd = p - dp;

		if((dd > 3)||(dd < 1))
			return(-1);
		delay = atoi(dp);
		if(dd == 1)
			delay *= 100;
		else if(dd == 2)
			delay *= 10;
		delay *= 1000;
	}
	delay += atol(ds)*1000000;
	return(delay);
}

LOCAL char *ReadLine()
{
	char		c;
	static char	lbuff[50];
	char		*lp = lbuff;

	noraw();
	echo();
	while((c = getch()) >= 0)
	{
		if(c == '\n')
		{
			*lp = 0;
			noecho();
			raw();
			return(lbuff);
		}
		if(c == 0x7f)
		{
			if(lp == &lbuff[0])
				continue;
			--lp;
			printw("\b");
			clrtoeol();
			refresh();
			continue;
		}
		*lp++ = c;
		if(lp == &lbuff[sizeof(lbuff)-2])
			break;
	}
	noecho();
	raw();
	return(NULL);
}

/*
 ********************* Main Program *******************
 */

#define STATLINES (6)

LOCAL int
do_top()
{
	int			nread;
	int			npr;

	nlist("/vmunix", nl);
	if(nl[0].n_type == 0)
	{
		fprintf(stderr, "no /vmunix namelist\n");
		return(1);
	}

	/*
	 * Open files required for obtaining system information:
	 * /dev/kmem - kernel symbols and proc array
	 * /dev/mem  - incore process user struct and stack
	 * /dev/swap - swapped process user struct and stack
	 */
	if((kmem = open(kmemf, 0)) < 0)
	{
		perror(kmemf);
		return(1);
	}
	if((mem = open(memf, 0)) < 0)
	{
		perror(memf);
		return(1);
	}
	if((swap = open(swapf, 0)) < 0)
	{
		perror(swapf);
		return(1);
	}

        if(getksyms("/unix") < 0)
	{
		fprintf(stderr, "Can't read kernel symbols - aborting\r\n");
		return(1);
	}

	/* find number of procs */

	if(nproc_sym == 0)
	{
		fprintf(stderr, "nproc not in namelist\r\n");
		return(-1);
	}

	if(readat(kmem, (off_t) nproc_sym, (char *)&nproc,sizeof(nproc)) <0)
	{
		perror(kmemf);
		return(-1);
	}
	/* find value of hz */
	if(readat(kmem, (off_t) hz_sym, (char *)&hz,sizeof(hz)) < 0)
	{
		perror(kmemf);
		return(-1);
	}
	fhz = hz * 1.0;
	memset(untab, 0, sizeof(untab));
	/*
	 * allocate an array to hold processess information
	 */
	ptsz = nproc * sizeof(struct proc);

	if((proctab = (struct proc *)malloc(ptsz))==NULL)
	{
		fprintf(stderr, "top: not enough memory for proc table\r\n");
		exit(1);
	}
	/*
	 * allocate an array to user struct information
	 */
	if((userdata=(struct udata_s *)calloc(nproc, sizeof(struct udata_s)))==NULL)
	{
		fprintf(stderr, "top: can't allocate %d bytes for saving info\r\n", nproc*sizeof(struct udata_s));
		exit(1);
	}
	/*
	 * allocate an indirection table for efficient sorting
	 */
	if( (procidx = (int *) malloc(nproc*sizeof(short))) == NULL)
	{
		fprintf(stderr, "top: cant allocate index table\r\n");
		return(1);
	}
	if((npr = update()) < 0)
		return(1);

	if((ut = fopen(_PATH_UTMP, "r")) == NULL)
	{
		perror(_PATH_UTMP);
		return(1);
	}

	while(1)
	{
		char c = keypress(upd_us);

		if(c != 0)
		{
			if(c == 'q')
			{
				move(LINES-1,0);
				clrtoeol();
				refresh();
				endwin();
				return(0);
			}

			if(c == 's')
			{
				char	*s;

				move(STATLINES-1,0);
				printw("Change delay from %4.3f to ",
					(upd_us *1.0)/1000000);
				clrtoeol();
				refresh();

				s = ReadLine();

				move(STATLINES-1,0);
				clrtoeol();
				move(0,0);
				refresh();

				if (s)
				{
					long newdelay;

					if((newdelay = GetDelay(s)) > 0)
					{
						upd_us = newdelay;
						continue;
					}
					if(*s == 0)
						continue;
				}

				move(STATLINES-1,0);
				printw("Unacceptable delay: %s", s);
				move(0,0);
				refresh();
				sleep(1);
				continue;
			}

			if(c == 'o')
			{
				char	*s;

				move(STATLINES-1,0);
				printw("Order: ");
				clrtoeol();
				refresh();

				s = ReadLine();

				move(STATLINES-1,0);
				clrtoeol();
				move(0,0);
				refresh();

				if (s)
				{
					int newsort;

					newsort = atoi(s);
					if ((newsort >= 0) && (newsort <= 10))
					{
						sort = newsort;
					}
					else
					{
						move(STATLINES-1,0);
						printw("Bad order");
						move(0,0);
						refresh();
						sleep(1);
					}
				}
			}

			if (c == 'I')
			{
				idle = 1-idle;
			}
#if !BADKEY_IGNORE

			move(LINES-1,0);
			printw("\nUnexpected command: '%c'", c);
			refresh();
			return(0);
/* otherwise invalid key commands just force an update */
#endif
		}
		if((npr = update()) < 0)
			return(1);

		erase();
		move(0,0);
		show_stats(npr);
		printw("\n");
		show_titles();
		show_procs(npr, LINES-(STATLINES+1));
		move(0,0);
		refresh();
	}
}

#if !TTY_RAW
LOCAL int sighand(sig, code, scp)
int			sig, code;
struct sigcontext	*scp;
{
	done = 1;
}
#endif /* TTY_RAW */

LOCAL void usage(p)
char *p;
{
	fprintf(stderr, "Usage: %s [opts]\n", p);
	fprintf(stderr, "Where options are\n");
	fprintf(stderr, "\t-h         show this summary\n");
	fprintf(stderr, "\t-s<delay>  delay in seconds between updates\n");
	fprintf(stderr, "\t-o<order>  select sort column\n");
	fprintf(stderr, "\t-I         do not show idle processes\n");
}

EXPORT int main(int argc, char *argv[])
{
	int 	status = 0;
	char	ch;
	char	*p;

	if (argc < 2)
	{
		if (argv[1] = getenv("TOP")) argc = 2;
	}

	while((ch = getopt(argc, argv, "hs:o:I")) != EOF)
	{
		switch(ch)
		{
			case 's':
				if((upd_us = GetDelay(optarg)) <= 0)
				{
					fprintf(stderr, "Invalid delay\n");
					exit(1);
				}
				break;		
			case 'o':
				sort = atoi(optarg);
				break;
			case 'I':
				idle = 0;
				break;
			default:
			case 'h':
				usage(argv[0]);
				exit(1);
		}
		argc -= optind;
		argv += optind;
	}

#if !TTY_RAW
	if(
		(signal(SIGHUP, sighand) == -1)
		||
		(signal(SIGINT, sighand) == -1)
	  )
	{
		perror("signal");
		exit(1);
	}
#endif /* TTY_RAW */

	gethostname(host, sizeof(host)-1);
	host[sizeof(host)-1] = 0;
	if (p = strchr(host, '.')) *p = 0;

	if(config_tty(1) < 0)
		return(1);

	if(do_top() != 0)
		status = 1;

	config_tty(0);
	if(done)
		fprintf(stdout, "*interrupt*\n");
	return(status);
}
