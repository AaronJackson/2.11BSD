#
/*
 * pxp - Pascal execution profiler
 *
 * Bill Joy UCB
 * Version 1.2 January 1979
 */

#include "0.h"

/*
 * Profile counter processing cluster
 *
 * This file contains all routines which do the hard work in profiling.
 *
 * The first group of routines (getit, getpmon, getcore, and pmread)
 * deal with extracting data from the pmon.out and (with more difficulty)
 * core files.
 *
 * The routines cnttab and prttab collect counters for
 * and print the summary table respectively.
 *
 * The routines "*cnt*" deal with manipulation of counters,
 * especially the "current" counter px.
 */
STATIC	struct pxcnt px;

/*
 * Table to record info
 * for procedure/function summary
 */
STATIC	struct pftab {
	long	pfcnt;
	int	pfline;
	char	*pfname;
	int	pflev;
} *zpf;

/*
 * Global variables
 */
STATIC	long *zbuf; 	/* Count buffer */
STATIC	int zcnt;	/* Number of counts */
STATIC	int zpfcnt;	/* Number of proc/funcs's */
STATIC	int gcountr;	/* Unique name generator */
STATIC	int zfil;	/* I/o unit for count data reads */
STATIC	int lastpf;	/* Total # of procs and funcs for consistency chk */

getit(fp)
	register char *fp;
{

	if (core)
		getcore(fp);
	else
		getpmon(fp);
}

/*
 * Setup monitor data buffer from pmon.out
 * style file whose name is fp.
 */
getpmon(fp)
	char *fp;
{
	register char *cp;
	int garbage;

	zfil = open(fp, 0);
	if (zfil < 0) {
		perror(fp);
		pexit(NOSTART);
	}
	if (pmread() < 0 || read(zfil, &garbage, 1) == 1) {
		Perror(fp, "Bad format for pmon.out style file");
		exit(1);
	}
	close(zfil);
	return;
}

STATIC	char nospcm[]	= "Not enough memory for count buffers\n";

pmnospac()
{

	write(2, nospcm, sizeof nospcm);
	pexit(NOSTART);
}

/*
 * Structure of the first few
 * items of a px core dump.
 */
STATIC	struct info {
	char	*off;		/* Self-reference for pure text */
	int	type;		/* 0 = non-pure text, 1 = pure text */
	char	*bp;		/* Core address of pxps struct */
} inf;

/*
 * First few words of the px
 * information structure.
 */
STATIC	struct pxps {
	char	*buf;
	int	cnt;
} pxp;

getcore(fp)
	char *fp;
{
	int pm;

	zfil = open(fp, 0);
	if (zfil < 0) {
		perror(fp);
		pexit(NOSTART);
	}
	if (lseek(zfil, (long)02000, 0) < 0)
		goto format;
	if (read(zfil, &inf, sizeof inf) < 0)
		goto format;
	if (inf.type != 0 && inf.type != 1)
		goto format;
	if (inf.type)
		inf.bp -= inf.off;
	if (lseek(zfil, (long)inf.bp + 02000, 0) < 0)
		goto format;
	if (read(zfil, &pxp, sizeof pxp) != sizeof pxp)
		goto format;
	if (pxp.buf == NIL) {
		Perror(fp, "No profile data in file");
		exit(1);
	}
	if (inf.type)
		pxp.buf -= inf.off;
	if (lseek(zfil, (long)pxp.buf + 02000, 0) < 0)
		goto format;
	if (pmread() < 0)
		goto format;
	close(zfil);
	return;
format:
	Perror(fp, "Not a Pascal system core file");
	exit(1);
}

pmread()
{
	register i;
	register char *cp;
	struct {
		int	no;
		int	no2;
		time_t	tvec;
	} zmagic;

	if (read(zfil, &zmagic, sizeof zmagic) != sizeof zmagic)
		return (-1);
	if (zmagic.no != 0426 || zmagic.no2)
		return (-1);
	ptvec = zmagic.tvec;
	if (read(zfil, &zcnt, 2) != 2)
		return (-1);
	if (read(zfil, &zpfcnt, 2) != 2)
		return (-1);
	cp = zbuf = alloc(i = zcnt * sizeof *zbuf);
	if (cp == -1)
		pmnospac();
	cp = zpf = alloc(zpfcnt * sizeof *zpf);
	if (cp == -1)
		pmnospac();
	if (read(zfil, zbuf, i) != i)
		return (-1);
	zbuf -= 2;
	return (0);
}

cnttab(s, no)
	char *s;
	int no;
{
	register struct pftab *pp;

	lastpf++;
	if (table == 0)
		return;
	if (no == zpfcnt)
		cPANIC();
	pp = &zpf[no];
	pp->pfname = s;
	pp->pfline = line;
	pp->pfcnt = nowcnt();
	pp->pflev = cbn;
}

prttab()
{
	register i, j;
	register struct pftab *zpfp;

	if (profile == 0 && table == 0)
		return;
	if (cnts != zcnt || lastpf != zpfcnt)
		cPANIC();
	if (table == 0)
		return;
	if (profile)
		printf("\f\n");
	header();
	printf("\n\tLine\t   Count\n\n");
	zpfp = zpf;
	for (i = 0; i < zpfcnt; i++) {
		printf("\t%4d\t%8ld\t", zpfp->pfline, zpfp->pfcnt);
		if (!justify)
			for (j = zpfp->pflev * unit; j > 1; j--)
				putchar(' ');
		printf("%s\n", zpfp->pfname);
		zpfp++;
	}
}

nowcntr()
{

	return (px.counter);
}

long nowcnt()
{

	return (px.ntimes);
}

long cntof(pxc)
	struct pxcnt *pxc;
{

	if (profile == 0 && table == 0)
		return;
	return (pxc->ntimes);
}

setcnt(l)
	long l;
{

	if (profile == 0 && table == 0)
		return;
	px.counter = --gcountr;
	px.ntimes = l;
	px.gos = gocnt;
	px.printed = 0;
}

savecnt(pxc)
	register struct pxcnt *pxc;
{

	if (profile == 0 && table == 0)
		return;
	pxc->ntimes = px.ntimes;
	pxc->counter = px.counter;
	pxc->gos = px.gos;
	pxc->printed = 1;
}

rescnt(pxc)
	register struct pxcnt *pxc;
{

	if (profile == 0 && table == 0)
		return;
	px.ntimes = pxc->ntimes;
	px.counter = pxc->counter;
	px.gos = gocnt;
	px.printed = pxc->printed;
	return (gocnt != pxc->gos);
}

getcnt()
{

	if (profile == 0 && table == 0)
		return;
	if (cnts == zcnt)
		cPANIC();
	px.counter = cnts;
	px.ntimes = zbuf[cnts];
	px.gos = gocnt;
	px.printed = 0;
	++cnts;
}

unprint()
{

	px.printed = 0;
}

/*
 * Control printing of '|'
 * when profiling.
 */
STATIC	char	nobar;

baroff()
{

	nobar = 1;
}

baron()
{

	nobar = 0;
}

/*
 * Do we want cnt and/or '|' on this line ?
 *	1 = count and '|'
 *	0 = only '|'
 *     -1 = spaces only
 */
shudpcnt()
{

	register i;

	if (nobar)
		return (-1);
	i = px.printed;
	px.printed = 1;
	return (i == 0);
}

STATIC	char mism[]	= "Program and counter data do not correspond\n";

cPANIC()
{

	printf("cnts %d zcnt %d, lastpf %d zpfcnt %d\n",
		cnts, zcnt, lastpf, zpfcnt);
	flush();
	write(2, mism, sizeof mism);
	pexit(ERRS);
}
