/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if	!defined(LINT) && defined(DOSCCS)
char copyright[] =
"@(#) Copyright (c) 1985 Regents of the University of California.\n\
 All rights reserved.\n";

static char sccsid[] = "@(#)date.c	4.20.1 (2.11BSD) 96/7/10";
#endif

/*
 * Date - print and set date
 */

#include <sys/param.h>
#include <sys/time.h>
#include <sys/file.h>
#include <errno.h>
#include <syslog.h>
#include <utmp.h>
#include <tzfile.h>
#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>

#define	WTMP		"/usr/adm/wtmp"
#define	ATOI2(ar)	(ar[0] - '0') * 10 + (ar[1] - '0'); ar += 2;

static struct timeval	tv;
static int	retval;

static int	dmsize[] =
	{ -1, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static struct utmp	wtmp[2] = {
	{ "|", "", "", 0 },
	{ "{", "", "", 0 }
};

main(argc,argv)
	int	argc;
	char	**argv;
{
	static char	usage[] = "usage: date [-nu] [-d dst] [-t timezone] [yymmddhhmm[.ss]]\n";
	struct timezone	tz;
	char	*ap,			/* time string */
		*tzn;			/* time zone */
	int	ch,			/* getopts char */
		uflag,			/* do it in GMT */
		nflag,			/* only set time locally */
		wf;			/* wtmp file descriptor */
	char	*username;

	nflag = uflag = 0;
	tz.tz_dsttime = tz.tz_minuteswest = 0;
	while ((ch = getopt(argc,argv,"d:nut:")) != EOF)
		switch((char)ch) {
		case 'd':
			tz.tz_dsttime = atoi(optarg) ? 1 : 0;
			break;
		case 'n':
			nflag = 1;
			break;
		case 't':
			tz.tz_minuteswest = atoi(optarg);
			break;
		case 'u':
			uflag = 1;
			break;
		default:
			fputs(usage,stderr);
			exit(1);
		}
	argc -= optind;
	argv += optind;

	if (argc > 1) {
		fputs(usage,stderr);
		exit(1);
	}

	if ((tz.tz_minuteswest || tz.tz_dsttime) &&
	    settimeofday((struct timeval *)NULL,&tz)) {
		perror("settimeofday");
		retval = 1;
		goto display;
	}

	if (gettimeofday(&tv,&tz)) {
		perror("gettimeofday");
		exit(1);
	}

	if (!argc)
		goto display;

	wtmp[0].ut_time = tv.tv_sec;
	if (gtime(*argv)) {
		fputs(usage,stderr);
		retval = 1;
		goto display;
	}

	if (!uflag) {		/* convert to GMT assuming local time */
		tv.tv_sec += (long)tz.tz_minuteswest * SECS_PER_MIN;
				/* now fix up local daylight time */
		if (localtime((time_t *)&tv.tv_sec)->tm_isdst)
			tv.tv_sec -= SECS_PER_HOUR;
	}
	if (nflag || !netsettime(tv)) {
		if (settimeofday(&tv,(struct timezone *)0)) {
			perror("settimeofday");
			retval = 1;
			goto display;
		}
		if ((wf = open(WTMP,O_WRONLY|O_APPEND)) < 0)
			fputs("date: can't write wtmp file.\n",stderr);
		else {
			(void)time((time_t *)&wtmp[1].ut_time);
			/*NOSTRICT*/
			(void)write(wf,(char *)wtmp,sizeof(wtmp));
			(void)close(wf);
		}
	}

	username = getlogin();
	if (!username || *username == '\0')	/* single-user or no tty */
		username = "root";
	syslog(LOG_AUTH | LOG_NOTICE,"date set by %s",username);

display:
	if (gettimeofday(&tv,(struct timezone *)0)) {
		perror("gettimeofday");
		exit(1);
	}
	if (uflag) {
		ap = asctime(gmtime((time_t *)&tv.tv_sec));
		tzn = "GMT";
	}
	else {
		struct tm	*tp;

		tp = localtime((time_t *)&tv.tv_sec);
		ap = asctime(tp);
		tzn = tp->tm_zone;
	}
	printf("%.20s%s%s",ap,tzn,ap + 19);
	exit(retval);
}

/*
 * gtime --
 *	convert user's time into number of seconds
 */
static
gtime(ap)
	register char	*ap;		/* user argument */
{
	register int	year, month;
	register char	*C;		/* pointer into time argument */
	struct tm	*L;
	int	day, hour, mins, secs;

	for (secs = 0, C = ap;*C;++C) {
		if (*C == '.') {		/* seconds provided */
			if (strlen(C) != 3)
				return(1);
			*C = NULL;
			secs = (C[1] - '0') * 10 + (C[2] - '0');
			break;
		}
		if (!isdigit(*C))
			return(-1);
	}

	L = localtime((time_t *)&tv.tv_sec);
	year = L->tm_year;			/* defaults */
	month = L->tm_mon + 1;
	day = L->tm_mday;

	switch ((int)(C - ap)) {		/* length */
		case 10:			/* yymmddhhmm */
			year = ATOI2(ap);
		case 8:				/* mmddhhmm */
			month = ATOI2(ap);
		case 6:				/* ddhhmm */
			day = ATOI2(ap);
		case 4:				/* hhmm */
			hour = ATOI2(ap);
			mins = ATOI2(ap);
			break;
		default:
			return(1);
	}

	if (*ap || month < 1 || month > 12 || day < 1 || day > 31 ||
	     mins < 0 || mins > 59 || secs < 0 || secs > 59)
		return(1);
	if (hour == 24) {
		++day;
		hour = 0;
	}
	else if (hour < 0 || hour > 23)
		return(1);

	tv.tv_sec = 0;
	year += TM_YEAR_BASE;
/* If year < EPOCH_YEAR, assume it's in the next century and
   the system has not yet been patched to move TM_YEAR_BASE up yet */
	if (year < EPOCH_YEAR)
		year += 100;
	if (isleap(year) && month > 2)
		++tv.tv_sec;
	for (--year;year >= EPOCH_YEAR;--year)
		tv.tv_sec += isleap(year) ? DAYS_PER_LYEAR : DAYS_PER_NYEAR;
	while (--month)
		tv.tv_sec += dmsize[month];
	tv.tv_sec += day - 1;
	tv.tv_sec = HOURS_PER_DAY * tv.tv_sec + hour;
	tv.tv_sec = MINS_PER_HOUR * tv.tv_sec + mins;
	tv.tv_sec = SECS_PER_MIN * tv.tv_sec + secs;
	return(0);
}

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define TSPTYPES
#include <protocols/timed.h>

#define WAITACK		2	/* seconds */
#define WAITDATEACK	5	/* seconds */

/*
 * Set the date in the machines controlled by timedaemons
 * by communicating the new date to the local timedaemon. 
 * If the timedaemon is in the master state, it performs the
 * correction on all slaves.  If it is in the slave state, it
 * notifies the master that a correction is needed.
 * Returns 1 on success, 0 on failure.
 */
static
netsettime(ntv)
	struct timeval ntv;
{
	int s, length, port, timed_ack, found, err;
	long waittime;
	fd_set ready;
	char hostname[MAXHOSTNAMELEN];
	struct timeval tout;
	struct servent *sp;
	struct tsp msg;
	struct sockaddr_in sin, dest, from;

	sp = getservbyname("timed", "udp");
	if (sp == 0) {
		fputs("udp/timed: unknown service\n",stderr);
		retval = 2;
		return (0);
	}
	dest.sin_port = sp->s_port;
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = htonl((u_long)INADDR_ANY);
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		if (errno == EPROTONOSUPPORT)
			return(0);
		perror("date: socket");
		goto bad;
	}
	bzero((char *)&sin, sizeof (sin));
	sin.sin_family = AF_INET;
	for (port = IPPORT_RESERVED - 1; port > IPPORT_RESERVED / 2; port--) {
		sin.sin_port = htons((u_short)port);
		if (bind(s, (struct sockaddr *)&sin, sizeof (sin)) >= 0)
			break;
		if (errno != EADDRINUSE) {
			if (errno != EADDRNOTAVAIL)
				perror("date: bind");
			goto bad;
		}
	}
	if (port == IPPORT_RESERVED / 2) {
		fputs("date: All ports in use\n",stderr);
		goto bad;
	}
	msg.tsp_type = TSP_SETDATE;
	msg.tsp_vers = TSPVERSION;
	if (gethostname(hostname, sizeof (hostname))) {
		perror("gethostname");
		goto bad;
	}
	(void) strncpy(msg.tsp_name, hostname, sizeof (hostname));
	msg.tsp_seq = htons((u_short)0);
	msg.tsp_time.tv_sec = htonl((u_long)ntv.tv_sec);
	msg.tsp_time.tv_usec = htonl((u_long)ntv.tv_usec);
	length = sizeof (struct sockaddr_in);
	if (connect(s, &dest, length) < 0) {
		perror("date: connect");
		goto bad;
	}
	if (send(s, (char *)&msg, sizeof (struct tsp), 0) < 0) {
		if (errno != ECONNREFUSED)
			perror("date: send");
		goto bad;
	}
	timed_ack = -1;
	waittime = WAITACK;
loop:
	tout.tv_sec = waittime;
	tout.tv_usec = 0;
	FD_ZERO(&ready);
	FD_SET(s, &ready);
	found = select(FD_SETSIZE, &ready, (fd_set *)0, (fd_set *)0, &tout);
	length = sizeof(err);
	if (getsockopt(s, SOL_SOCKET, SO_ERROR, (char *)&err, &length) == 0
	    && err) {
		errno = err;
		if (errno != ECONNREFUSED)
			perror("date: send (delayed error)");
		goto bad;
	}
	if (found > 0 && FD_ISSET(s, &ready)) {
		length = sizeof (struct sockaddr_in);
		if (recvfrom(s, (char *)&msg, sizeof (struct tsp), 0, &from,
		    &length) < 0) {
			if (errno != ECONNREFUSED)
				perror("date: recvfrom");
			goto bad;
		}
		msg.tsp_seq = ntohs(msg.tsp_seq);
		msg.tsp_time.tv_sec = ntohl(msg.tsp_time.tv_sec);
		msg.tsp_time.tv_usec = ntohl(msg.tsp_time.tv_usec);
		switch (msg.tsp_type) {

		case TSP_ACK:
			timed_ack = TSP_ACK;
			waittime = WAITDATEACK;
			goto loop;

		case TSP_DATEACK:
			(void)close(s);
			return (1);

		default:
			fprintf(stderr,
			    "date: Wrong ack received from timed: %s\n", 
			    tsptype[msg.tsp_type]);
			timed_ack = -1;
			break;
		}
	}
	if (timed_ack == -1)
		fputs("date: Can't reach time daemon, time set locally.\n",
		    stderr);
bad:
	(void)close(s);
	retval = 2;
	return (0);
}
