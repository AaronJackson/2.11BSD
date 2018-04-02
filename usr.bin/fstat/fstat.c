/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

#if	defined(DOSCCS) && !defined(lint)
char copyright[] =
"@(#) Copyright (c) 1987 Regents of the University of California.\n\
 All rights reserved.\n";

static char sccsid[] = "@(#)fstat.c	5.11.1 (2.11BSD GTE) 12/31/93";
#endif

/*
 *  fstat 
 */
#include <sys/param.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/text.h>
#include <sys/stat.h>
#include <sys/inode.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/domain.h>
#include <sys/protosw.h>
#include <sys/unpcb.h>
#include <sys/vmmac.h>
#define	KERNEL
#include <sys/file.h>
#undef	KERNEL
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_pcb.h>
#include <stdio.h>
#include <ctype.h>
#include <nlist.h>
#include <pwd.h>

#ifdef	ULTRIX
		/* UFS -> GFS */
#    define	inode	gnode
#    define	x_iptr	x_gptr
#    define	i_dev	g_dev
#    define	i_number g_number
#    define	i_mode	g_mode
#    define	i_size	g_size
#endif

#define	N_KMEM	"/dev/kmem"
#define	N_MEM	"/dev/mem"
#define	N_SWAP	"/dev/drum"
#define	N_UNIX	"/vmunix"

#define	TEXT	-2
#define	WD	-1

typedef struct devs {
	struct devs	*next;
	dev_t	dev;
	int	inum;
	char	*name;
} DEVS;
DEVS	*devs;

#ifdef pdp11
static struct nlist nl[] = {
	{ "_proc" },
#define	X_PROC		0
	{ "_nproc" },
#define	X_NPROC		1
	{ "_netdata" },
#define	X_NETDATA	2
	{ "" },
};
#else
static struct nlist nl[] = {
	{ "_proc" },
#define	X_PROC		0
	{ "_Usrptmap" },
#define	X_USRPTMA	1
	{ "_nproc" },
#define	X_NPROC		2
	{ "_usrpt" },
#define	X_USRPT		3
	{ "" },
};
#endif

struct proc	*mproc;
#ifndef pdp11
struct pte	*Usrptma, *usrpt;
#endif

union {
	struct	user user;
#ifdef pdp11
	char	upages[ctob(USIZE)];
#else
	char	upages[UPAGES][NBPG];
#endif
} user;

extern int	errno;
static int	fflg, vflg;
static int	kmem, mem, nproc, swap;
static char	*uname;
#ifdef pdp11
static off_t	netdata;
long	lgetw();
#endif

off_t	lseek();

main(argc, argv)
	int	argc;
	char	**argv;
{
	extern char *optarg;
	extern int optind;
	register struct passwd *passwd;
	register int pflg, pid, uflg, uid;
	int ch, size;
	struct passwd *getpwnam(), *getpwuid();
	long lgetw();
	char *malloc();

	pflg = uflg = 0;
	while ((ch = getopt(argc, argv, "p:u:v")) != EOF)
		switch((char)ch) {
		case 'p':
			if (pflg++)
				usage();
			if (!isdigit(*optarg)) {
				fputs("fstat: -p option requires a process id.\n", stderr);
				usage();
			}
			pid = atoi(optarg);
			break;
		case 'u':
			if (uflg++)
				usage();
			if (!(passwd = getpwnam(optarg))) {
				fprintf(stderr, "%s: unknown uid\n", optarg);
				exit(1);
			}
			uid = passwd->pw_uid;
			uname = passwd->pw_name;
			break;
		case 'v':	/* undocumented: print read error messages */
			vflg++;
			break;
		case '?':
		default:
			usage();
		}

	if (*(argv += optind)) {
		for (; *argv; ++argv) {
			if (getfname(*argv))
				fflg = 1;
		}
		if (!fflg)	/* file(s) specified, but none accessable */
			exit(1);
	}

	openfiles();

	if (nlist(N_UNIX, nl) == -1 || !nl[0].n_type) {
		fprintf(stderr, "%s: No namelist\n", N_UNIX);
		exit(1);
	}
#ifdef pdp11
	if (nl[X_NETDATA].n_type) {
		netdata = lgetw((off_t)nl[X_NETDATA].n_value);
		netdata = ctob(netdata);
	}
#else
	Usrptma = (struct pte *)nl[X_USRPTMA].n_value;
	usrpt = (struct pte *) nl[X_USRPT].n_value;
#endif
	nproc = (int)lgetw((off_t)nl[X_NPROC].n_value);

#ifdef pdp11
	(void)lseek(kmem, (off_t)nl[X_PROC].n_value, L_SET);
#else
	(void)lseek(kmem, lgetw((off_t)nl[X_PROC].n_value), L_SET);
#endif
	size = nproc * sizeof(struct proc);
	if ((mproc = (struct proc *)malloc((u_int)size)) == NULL) {
		fprintf(stderr, "fstat: out of space.\n");
		exit(1);
	}
	if (read(kmem, (char *)mproc, size) != size)
		rerr1("proc table", N_KMEM);

	printf("USER\t CMD\t      PID    FD\tDEVICE\tINODE\t  SIZE TYPE%s\n",
	    fflg ? " NAME" : "");
	for (; nproc--; ++mproc) {
		if (mproc->p_stat == 0)
			continue;
		if (pflg && mproc->p_pid != pid)
			continue;
		if (uflg)  {
			if (mproc->p_uid != uid)
				continue;
		}
		else
			uname = (passwd = getpwuid(mproc->p_uid)) ?
			    passwd->pw_name : "unknown";
		if (mproc->p_stat != SZOMB && getu() == 0)
			continue;
		dotext();
		readf();
	}
	exit(0);
}

#ifndef pdp11
static
getu()
{
	struct pte *pteaddr, apte;
	struct pte arguutl[UPAGES+CLSIZE];
	register int i;
	int ncl;

	if ((mproc->p_flag & SLOAD) == 0) {
		if (swap < 0)
			return(0);
		(void)lseek(swap, (off_t)dtob(mproc->p_swaddr), L_SET);
		if (read(swap, (char *)&user.user, sizeof(struct user))
		    != sizeof(struct user)) {
			fprintf(stderr, "fstat: can't read u for pid %d from %s\n", mproc->p_pid, N_SWAP);
			return(0);
		}
		return(1);
	}
	pteaddr = &Usrptma[btokmx(mproc->p_p0br) + mproc->p_szpt - 1];
	(void)lseek(kmem, (off_t)pteaddr, L_SET);
	if (read(kmem, (char *)&apte, sizeof(apte)) != sizeof(apte)) {
		printf("fstat: can't read indir pte to get u for pid %d from %s\n", mproc->p_pid, N_SWAP);
		return(0);
	}
	(void)lseek(mem, (off_t)ctob(apte.pg_pfnum+1) - (UPAGES+CLSIZE)
	    * sizeof(struct pte), L_SET);
	if (read(mem, (char *)arguutl, sizeof(arguutl)) != sizeof(arguutl)) {
		printf("fstat: can't read page table for u of pid %d from %s\n", mproc->p_pid, N_KMEM);
		return(0);
	}
	ncl = (sizeof(struct user) + NBPG*CLSIZE - 1) / (NBPG*CLSIZE);
	while (--ncl >= 0) {
		i = ncl * CLSIZE;
		(void)lseek(mem, (off_t)ctob(arguutl[CLSIZE+i].pg_pfnum), L_SET);
		if (read(mem, user.upages[i], CLSIZE*NBPG) != CLSIZE*NBPG) {
			printf("fstat: can't read page %u of u of pid %d from %s\n", arguutl[CLSIZE+i].pg_pfnum, mproc->p_pid, N_MEM);
			return(0);
		}
	}
	return(1);
}
#else
static
getu()
{

	if ((mproc->p_flag & SLOAD) == 0) {
		if (swap < 0)
			return(0);
		(void)lseek(swap, (off_t)(mproc->p_addr)<<9, L_SET);
		if (read(swap, (char *)&user.user, sizeof(struct user))
		    != sizeof(struct user)) {
			fprintf(stderr, "fstat: can't read u for pid %d from %s\n", mproc->p_pid, N_SWAP);
			return(0);
		}
		return(1);
	}
	(void)lseek(mem, (off_t)ctob((off_t)mproc->p_addr), L_SET);
	if (read(mem, &user.user, sizeof(user.user))!=sizeof(user.user)) {
		printf("fstat: can't read page table for u of pid %d from %s\n", mproc->p_pid, N_MEM);
		return(0);
	}
	return(1);
}
#endif

static
dotext()
{
	struct text	text;

#ifdef pdp11
	if (!mproc->p_textp)
		return;
#endif
	(void)lseek(kmem, (off_t)mproc->p_textp, L_SET);
	if (read(kmem, (char *) &text, sizeof(text)) != sizeof(text)) {
		rerr1("text table", N_KMEM);
		return;
	}
	if (text.x_flag)
		itrans(DTYPE_INODE, text.x_iptr, TEXT);
}

static
itrans(ftype, g, fno)
	int ftype, fno;
	struct inode	*g;		/* if ftype is inode */
{
	struct inode inode;
	dev_t idev;
	char *comm, *itype();
	char *name = (char *)NULL;	/* set by devmatch() on a match */

	if (g || fflg) {
		(void)lseek(kmem, (off_t)g, L_SET);
		if (read(kmem, (char *)&inode, sizeof(inode)) != sizeof(inode)) {
			rerr2(errno, (int)g, "inode");
			return;
		}
		idev = inode.i_dev;
		if (fflg && !devmatch(idev, inode.i_number, &name))
			return;
	}
	if (mproc->p_pid == 0)
		comm = "swapper";
#ifndef pdp11
	else if (mproc->p_pid == 2)
		comm = "pagedaemon";
#endif
	else
		comm = user.user.u_comm;
	printf("%-8.8s %-10.10s %5d  ", uname, comm, mproc->p_pid);

	switch(fno) {
	case WD:
		printf("  wd"); break;
	case TEXT:
		printf("text"); break;
	default:
		printf("%4d", fno);
	}

	if (g == 0) {
		printf("* (deallocated)\n");
		return;
	}

	switch(ftype) {
	case DTYPE_INODE:
#ifdef pdp11
	case DTYPE_PIPE:
#endif
		printf("\t%2d, %2d\t%5lu\t%6ld\t%3s %s\n", major(inode.i_dev),
		    minor(inode.i_dev), (long)inode.i_number,
		    inode.i_mode == IFSOCK ? 0L : inode.i_size,
#ifdef pdp11
		    ftype == DTYPE_PIPE ? "pip" :
#endif
		    itype(inode.i_mode), name ? name : "");
		break;
	case DTYPE_SOCKET:
		socktrans((struct socket *)g);
		break;
#ifdef DTYPE_PORT
	case DTYPE_PORT:
		printf("* (fifo / named pipe)\n");
		break;
#endif
	default:
		printf("* (unknown file type)\n");
	}
}

static char *
itype(mode)
	u_short mode;
{
	switch(mode & IFMT) {
	case IFCHR:
		return("chr");
	case IFDIR:
		return("dir");
	case IFBLK:
		return("blk");
	case IFREG:
		return("reg");
	case IFLNK:
		return("lnk");
	case IFSOCK:
		return("soc");
	default:
		return("unk");
	}
	/*NOTREACHED*/
}

static
socktrans(sock)
	struct socket *sock;
{
	static char *stypename[] = {
		"unused",	/* 0 */
		"stream", 	/* 1 */
		"dgram",	/* 2 */
		"raw",		/* 3 */
		"rdm",		/* 4 */
		"seqpak"	/* 5 */
	};
#define	STYPEMAX 5
	struct socket	so;
	struct protosw	proto;
	struct domain	dom;
	struct inpcb	inpcb;
	struct unpcb	unpcb;
	int len;
	char dname[32], *strcpy();

	/* fill in socket */
#ifdef pdp11
	(void)lseek(mem, (off_t)sock + netdata, L_SET);
	if (read(mem, (char *)&so, sizeof(struct socket))
#else
	(void)lseek(kmem, (off_t)sock, L_SET);
	if (read(kmem, (char *)&so, sizeof(struct socket))
#endif
	    != sizeof(struct socket)) {
		rerr2(errno, (int)sock, "socket");
		return;
	}

	/* fill in protosw entry */
#ifdef pdp11
	(void)lseek(mem, (off_t)so.so_proto + netdata, L_SET);
	if (read(mem, (char *)&proto, sizeof(struct protosw))
#else
	(void)lseek(kmem, (off_t)so.so_proto, L_SET);
	if (read(kmem, (char *)&proto, sizeof(struct protosw))
#endif
	    != sizeof(struct protosw)) {
		rerr2(errno, (int)so.so_proto, "protosw");
		return;
	}

	/* fill in domain */
#ifdef pdp11
	(void)lseek(mem, (off_t)proto.pr_domain + netdata, L_SET);
	if (read(mem, (char *)&dom, sizeof(struct domain))
#else
	(void)lseek(kmem, (off_t)proto.pr_domain, L_SET);
	if (read(kmem, (char *)&dom, sizeof(struct domain))
#endif
	    != sizeof(struct domain)) {
		rerr2(errno, (int)proto.pr_domain, "domain");
		return;
	}

	/*
	 * grab domain name
	 * kludge "internet" --> "inet" for brevity
	 */
	if (dom.dom_family == AF_INET)
		(void)strcpy(dname, "inet");
	else {
#ifdef pdp11
		(void)lseek(mem, (off_t)dom.dom_name + netdata, L_SET);
		if ((len = read(mem, dname, sizeof(dname) - 1)) < 0) {
#else
		(void)lseek(kmem, (off_t)dom.dom_name, L_SET);
		if ((len = read(kmem, dname, sizeof(dname) - 1)) < 0) {
#endif
			rerr2(errno, (int)dom.dom_name, "char");
			dname[0] = '\0';
		}
		else
			dname[len] = '\0';
	}

	if ((u_short)so.so_type > STYPEMAX)
		printf("* (%s unk%d %x", dname, so.so_type, so.so_state);
	else
		printf("* (%s %s %x", dname, stypename[so.so_type],
		    so.so_state);

	/* 
	 * protocol specific formatting
	 *
	 * Try to find interesting things to print.  For tcp, the interesting
	 * thing is the address of the tcpcb, for udp and others, just the
	 * inpcb (socket pcb).  For unix domain, its the address of the socket
	 * pcb and the address of the connected pcb (if connected).  Otherwise
	 * just print the protocol number and address of the socket itself.
	 * The idea is not to duplicate netstat, but to make available enough
	 * information for further analysis.
	 */
	switch(dom.dom_family) {
	case AF_INET:
		getinetproto(proto.pr_protocol);
		if (proto.pr_protocol == IPPROTO_TCP ) {
			if (so.so_pcb) {
#ifdef pdp11
				(void)lseek(mem,(off_t)so.so_pcb+netdata,L_SET);
				if (read(mem, &inpcb, sizeof(struct inpcb))
#else
				(void)lseek(kmem, (off_t)so.so_pcb, L_SET);
				if (read(kmem, (char *)&inpcb, sizeof(struct inpcb))
#endif
				    != sizeof(struct inpcb)){
					rerr2(errno, (int)so.so_pcb, "inpcb");
					return;
				}
				printf(" %x", (int)inpcb.inp_ppcb);
			}
		}
		else if (so.so_pcb)
			printf(" %x", (int)so.so_pcb);
		break;
	case AF_UNIX:
		/* print address of pcb and connected pcb */
		if (so.so_pcb) {
			printf(" %x", (int)so.so_pcb);
#ifdef pdp11
			(void)lseek(mem, (off_t)so.so_pcb + netdata, L_SET);
			if (read(mem, (char *)&unpcb, sizeof(struct unpcb))
#else
			(void)lseek(kmem, (off_t)so.so_pcb, L_SET);
			if (read(kmem, (char *)&unpcb, sizeof(struct unpcb))
#endif
			    != sizeof(struct unpcb)){
				rerr2(errno, (int)so.so_pcb, "unpcb");
				return;
			}
			if (unpcb.unp_conn) {
				char shoconn[4], *cp;

				cp = shoconn;
				if (!(so.so_state & SS_CANTRCVMORE))
					*cp++ = '<';
				*cp++ = '-';
				if (!(so.so_state & SS_CANTSENDMORE))
					*cp++ = '>';
				*cp = '\0';
				printf(" %s %x", shoconn, (int)unpcb.unp_conn);
			}
		}
		break;
	default:
		/* print protocol number and socket address */
		printf(" %d %x", proto.pr_protocol, (int)sock);
	}
	printf(")\n");
}

/*
 * getinetproto --
 *	print name of protocol number
 */
static
getinetproto(number)
	int number;
{
	char *cp;

	switch(number) {
	case IPPROTO_IP:
		cp = "ip"; break;
	case IPPROTO_ICMP:
		cp ="icmp"; break;
	case IPPROTO_GGP:
		cp ="ggp"; break;
	case IPPROTO_TCP:
		cp ="tcp"; break;
	case IPPROTO_EGP:
		cp ="egp"; break;
	case IPPROTO_PUP:
		cp ="pup"; break;
	case IPPROTO_UDP:
		cp ="udp"; break;
	case IPPROTO_IDP:
		cp ="idp"; break;
	case IPPROTO_RAW:
		cp ="raw"; break;
	default:
		printf(" %d", number);
		return;
	}
	printf(" %s", cp);
}

static
readf()
{
	struct file lfile;
	int i;

	itrans(DTYPE_INODE, user.user.u_cdir, WD);
	for (i = 0; i < NOFILE; i++) {
		if (user.user.u_ofile[i] == 0)
			continue;
		(void)lseek(kmem, (off_t)user.user.u_ofile[i], L_SET);
		if (read(kmem, (char *)&lfile, sizeof(lfile))
		    != sizeof(lfile)) {
			rerr1("file", N_KMEM);
			continue;
		}
		itrans(lfile.f_type, (struct inode *)lfile.f_data, i);
	}
}

static
devmatch(idev, inum, name)
	dev_t idev;
	ino_t inum;
	char  **name;
{
	register DEVS *d;

	for (d = devs; d; d = d->next)
		if (d->dev == idev && (d->inum == 0 || d->inum == inum)) {
			*name = d->name;
			return(1);
		}
	return(0);
}

static
getfname(filename)
	char *filename;
{
	struct stat statbuf;
	DEVS *cur;
	char *malloc();

	if (stat(filename, &statbuf)) {
		perror(filename);
		return(0);
	}
	if ((cur = (DEVS *)malloc(sizeof(DEVS))) == NULL) {
		fprintf(stderr, "fstat: out of space.\n");
		exit(1);
	}
	cur->next = devs;
	devs = cur;

	/* if file is block special, look for open files on it */
	if ((statbuf.st_mode & S_IFMT) != S_IFBLK) {
		cur->inum = statbuf.st_ino;
		cur->dev = statbuf.st_dev;
	}
	else {
		cur->inum = 0;
		cur->dev = statbuf.st_rdev;
	}
	cur->name = filename;
	return(1);
}

static
openfiles()
{
	if ((kmem = open(N_KMEM, O_RDONLY, 0)) < 0) {
		perror(N_KMEM);
		exit(1);
	}
	if ((mem = open(N_MEM, O_RDONLY, 0)) < 0) {
		perror(N_MEM);
		exit(1);
	}
	if ((swap = open(N_SWAP, O_RDONLY, 0)) < 0) {
		perror(N_SWAP);
		exit(1);
	}
}

static
rerr1(what, fromwhat)
	char *what, *fromwhat;
{
	if (vflg)
		printf("fstat: error reading %s from %s", what, fromwhat);
}

static
rerr2(err, address, what)
	int err, address;
	char *what;
{
	if (vflg)
		printf("error %d reading %s at %x from kmem\n", errno, what, address);
}

static long
lgetw(loc)
	off_t loc;
{
#ifdef pdp11
	u_short word;
#else
	long word;
#endif

	(void)lseek(kmem, (off_t)loc, L_SET);
	if (read(kmem, (char *)&word, sizeof(word)) != sizeof(word))
		rerr2(errno, (int)loc, "word");
	return((long)word);
}

static
usage()
{
	fputs("usage: fstat [-v] [-u user] [-p pid] [filename ...]\n", stderr);
	exit(1);
}
