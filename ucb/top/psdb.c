/*
 * Load the process status cache saved for the 'ps' program at boot
 * 
 * The ps data cache contains:
 *  
 * KernelName	MAXPATHLEN+1	Name of cached Kernel
 * nllen	int		namelist structure count
 * nttys	int		ttys structure count
 * nchans	int		wchan structure count
 * nl		nllen*struct	proc,npoc and hz namelist entries
 * ttlist	nttys*struct	terminal information
 * wclist	nchans*struct	wait channel information
 *
 */
#include <stdio.h>	/* fopen/fread.. */
#include <a.out.h>	/* struct nlist */
#include <sys/file.h>	/* lseek */
#include <sys/param.h>	/* MAXPATHLEN */
#include "psdb.h"

#define LOCAL 	static
#define EXPORT	/**/

#define MAXTTYS         160     /* 128 plus a few extra */

#define X_PROC          0
#define X_NPROC         1
#define X_HZ            2

LOCAL char		*psdbf	= "/var/run/psdatabase";
LOCAL struct nlist	nl[4];
/*
 * Returned information
 */
EXPORT u_int		proc_sym	= 0;
EXPORT u_int		hz_sym		= 0;
EXPORT u_int		nproc_sym	= 0;

EXPORT int		nttys		= 0;
EXPORT ttab_t		*ttlist		= NULL;

EXPORT int		nchans		= 0;
EXPORT wc_t 		*wclist		= NULL;

void *calloc();

LOCAL char *readpsdb()
{
	static	char	unamebuf[MAXPATHLEN+1];
	char		*p = unamebuf;
	register FILE	*fp;
	int		nllen;

	if((fp = fopen(psdbf, "r")) == NULL)
	{
		perror(psdbf);
		return(NULL);
	}

	while ((*p= getc(fp)) != '\0')
		p++;

	if(
	    (fread((char *)&nllen,	sizeof nllen, 1, fp) != 1)
	    ||
	    (fread((char *)&nttys,	sizeof nttys, 1, fp) != 1)
	    ||
	    (fread((char *)&nchans,	sizeof nchans, 1, fp) != 1)
	    ||
	    (fread((char *)nl,	sizeof(struct nlist), nllen, fp) != nllen)
	   )
	{
		perror(psdbf);
		return(NULL);
	}

	if((ttlist = (ttab_t *) calloc((unsigned)nttys, sizeof(ttab_t))) == NULL)
	{
		fputs("Too many tty names\n",stderr);
		return(NULL);
	}
	if(fread((char *)ttlist,	sizeof(ttab_t), nttys, fp) != nttys)
	{
		perror(psdbf);
		return(NULL);
	}

	if((wclist = (wc_t *)calloc((unsigned)nchans, sizeof(wc_t))) == NULL)
	{
		fputs("Too many wchan symbols\n",stderr);
		return(NULL);
	}

	if(fread((char *) wclist, sizeof(wc_t), nchans, fp) != nchans)
	{
		perror(psdbf);
		return(NULL);
	}
	(void) fclose(fp);
	return(unamebuf);
}

EXPORT int getksyms(kname)
char	*kname;
{
	char		*uname;

	if((uname = readpsdb()) == NULL)
		return(-1);

	if(strcmp(uname,kname) != 0)
	{
		free((char *)wclist);
		nchans = 0;
		return(-1);
	}

	proc_sym	= nl[X_PROC].n_value;
	nproc_sym	= nl[X_NPROC].n_value;
	hz_sym		= nl[X_HZ].n_value;
	return(0);
}
