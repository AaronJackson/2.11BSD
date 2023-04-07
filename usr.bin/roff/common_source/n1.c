#if	!defined(lint) && defined(DOSCCS)
static char sccsid[] = "@(#)n1.c	4.9 (2.11BSD) 2020/3/24";
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <setjmp.h>
#include <sgtty.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#ifdef __BSD2_11__
#include <paths.h>
#endif

#include "tdef.h"

extern
#include "d.h"
extern
#include "v.h"
#ifdef NROFF
#include "tw.h"
#endif
#include "sdef.h"
jmp_buf sjbuf;
/*
troff1.c

consume options, initialization, main loop,
input routines, escape function calling
*/

int	inchar[LNSIZE], *pinchar = inchar;	/* XXX */
extern struct s *frame, *stk, *nxf;
extern struct s *ejl;


extern int stdi;
extern int waitf;
extern int nofeed;
extern int quiet;
extern int ascii;
extern int npn;
extern int xflg;
extern int stop;
extern char ibuf[IBUFSZ];
extern char *ibufp;
extern char *eibuf;
extern int cbuf[NC];
extern int *cp;
extern int *vlist;
extern int nx;
extern int mflg;
extern int ch;
extern int pto;
extern int pfrom;
extern int cps;
extern int ibf;
extern int ttyod;
extern struct sgttyb ttys;
extern int iflg;
extern int init;
extern int rargc;
extern char **argp;
extern char trtab[256];
extern int lgf;
extern int copyf;
extern int eschar;
extern int ch0;
extern int cwidth;
extern int nlflg;
extern int *ap;
extern int donef;
extern int nflush;
extern int nchar;
extern int rchar;
extern int nfo;
extern int ifile;
extern int fc;
extern int padc;
extern int tabc;
extern int dotc;
extern int raw;
extern int tabtab[NTAB];
extern char nextf[];
extern int ifl[NSO];
extern int ifi;
extern int flss;
extern int lg;
extern char ptname[];
extern int print;
extern int nonumb;
extern int pnlist[];
extern int *pnp;
extern int trap;
extern int tflg;
extern int ejf;
extern int gflag;
extern int oline[];
extern int *olinep;
extern int dpn;
extern int noscale;
extern int pts;
extern int level;
extern int ttysave;
extern int tdelim;
extern int dotT;
extern int tabch, ldrch;
extern int eqflg;
extern no_out;
extern int hflg;
#ifndef NROFF
extern char codetab[];
#endif
extern int xxx;
int stopmesg;
long offl[NSO];
long ioff;
char *ttyp;
int ms[] = {31,28,31,30,31,30,31,31,30,31,30,31};
#ifndef NROFF
int acctf;
#endif

static void fpecatch(int), catch(int), kcatch(int);
static void setrpt(void);

static int iplb[NSO];
static int iplx[NSO];
static int xbufb;
FILE *fptid;
char *Mpath, *fontdir, *mfile, *devtab;

main(argc,argv)
int argc;
char **argv;
{
	char ebuf[EBUFSZ], obuf[OBUFSZ];
	char *p, *q;
	register i, j;

	setvbuf(stderr, ebuf, _IONBF, EBUFSZ);
	setvbuf(stdout, obuf, _IOFBF, OBUFSZ);
#ifdef NROFF
	fptid = stdout;
#endif

	signal(SIGHUP,catch);
	if(signal(SIGINT,catch) == SIG_IGN){
		signal(SIGHUP,SIG_IGN);
		signal(SIGINT,SIG_IGN);
		signal(SIGQUIT,SIG_IGN);
	}
	signal(SIGFPE,fpecatch);
	signal(SIGPIPE,catch);
	signal(SIGTERM,kcatch);
	init1(argv[0][0]);
options:
	while(--argc > 0 && (++argv)[0][0]=='-')
		switch(argv[0][1]){

		case 0:
			goto start;
		case 'i':
			stdi++;
			continue;
		case 'q':
			quiet++;
			if(gtty(0, &ttys) >= 0)
				ttysave = ttys.sg_flags;
			continue;
		case 'n':
			npn = cnum(&argv[0][2]);
			continue;
		case 'p':
			xflg = 0;
			cps = cnum(&argv[0][2]);
			continue;
		case 'S':
			stopmesg++;
			continue;
		case 's':
			if(!(stop = cnum(&argv[0][2])))stop++;
			continue;
		case 'r':
			vlist[findr(argv[0][2])] = cnum(&argv[0][3]);
			continue;
		case 'M':
			Mpath = &argv[0][2];
			continue;
		case 'm':
			mfile = &argv[0][2];
			mflg++;
			continue;
		case 'o':
			getpn(&argv[0][2]);
			continue;
		case 'F':
			fontdir = &argv[0][2];
			continue;
#ifdef NROFF
		case 'h':
			hflg++;
			continue;
		case 'z':
			no_out++;
			continue;
		case 'e':
			eqflg++;
			continue;
		case 'T':
			devtab = &argv[0][2];
			dotT++;
			continue;
#endif
#ifndef NROFF
		case 'z':
			no_out++;
		case 'a':
			ascii = 1;
			nofeed++;
		case 't':
			fptid = stdout;
			continue;
		case 'w':
			waitf = 2;
			continue;
		case 'f':
			nofeed++;
			continue;
		case 'x':
			xflg = 0;
			continue;
		case 'b':
			if(open(ptname,1) < 0)warnx("Busy.");
			else warnx("Available.");
			done3(0);
		case 'g':
			stop = gflag = 1;
			fptid = stdout;
			dpn = 0;
			continue;
#endif
		default:
			errx(1, "bad option %s", argv[0]);
			continue;
		}

	if (mfile)
		if (findfile(FTMAC, nextf, mfile) == NULL)
			err(1, "macro file");

start:
	argp = argv;
	rargc = argc;
	init2();
	setjmp(sjbuf);
loop:
	copyf = lgf = eblk.nb = nflush = nlflg = 0;
	if(ipb && (rbf0(ipb, ipx)==0) && ejf && (frame->pframe <= ejl)){
		nflush++;
		trap = 0;
		eject((struct s *)0);
		goto loop;
	}
	i = getch();
	if(eblk.pendt)
		goto lt;
	if((j = (i & CMASK)) == XPAR){
		copyf++;
		tflg++;
		for(;(i & CMASK) != '\n';)pchar(i = getch());
		tflg = 0;
		copyf--;
		goto loop;
	}
	if((j == eblk.cc) || (j == eblk.c2)){
		if(j == eblk.c2)eblk.nb++;
		copyf++;
		while(((j=((i=getch()) & CMASK)) == ' ') ||
			(j == '\t'));
		ch = i;
		copyf--;
		control(getrq(),1);
		flushi();
		goto loop;
	}
lt:
	ch = i;
	text();
	goto loop;
}

void
catch(){
/*
	warnx("Interrupt");
*/
	done3(01);
}

void
fpecatch()
{
	errx(1, "Floating Exception.");
}

void
kcatch(){
	signal(SIGTERM,SIG_IGN);
	done3(01);
}
#ifndef NROFF
acctg() {
	static char *acct_file = "/usr/adm/tracct";
	acctf = open(acct_file,1);
	setuid(getuid());
}
#endif
init1(a)
char a;
{
	register char *p;
	static char buf[] = "/tmp/taXXXXX";
	register i;

#ifndef NROFF
	acctg();/*open troff actg file while mode 4755*/
#endif
	/* create temp file for macros */
	if ((ibf = mkstemp(buf)) < 0)
		err(1, "Cannot create temp file");
	unlink(buf);	/* hide from file system */
	for(i=256; --i;)
		trtab[i]=i;
	trtab[UNPAD] = ' ';
	mchbits();
}
init2()
{
	register i,j;
	extern char *ttyname();

	ttyod = 2;
	if(((ttyp=ttyname(j=0)) != (char *)0) ||
	   ((ttyp=ttyname(j=1)) != (char *)0) ||
	   ((ttyp=ttyname(j=2)) != (char *)0)
	  );else ttyp = "notty";
	iflg = j;
	if(ascii)mesg(0);

#ifndef NROFF
	if (fptid == NULL) {
		do {
			if ((fptid = fopen(ptname, "w")))
				break;
			if (waitf) {
				warnx("Waiting for Typesetter.");
				sleep(15);
			}
		} while (waitf--);
		if (fptid == NULL) {
			warnx("Typesetter busy.");
			done3(-2);
		}
	}
#endif
#ifdef NROFF
	if (t.bset || t.breset) {
		gtty(1, &ttys);
		ttysave = ttys.sg_flags;

		ttys.sg_flags &= ~t.breset;
		ttys.sg_flags |= t.bset;
		stty(1, &ttys);
	}
	fprintf(fptid, "%s", t.twinit);
#endif
	ptinit();
	inithsh();

	if (lseek(ibf, EVBASE, 0) < 0)
		err(1, "lseek1");
	for(i=NEV; i--;)
		if (write(ibf,(char *)&eblk.block, sizeof(struct eblk)) < 0)
			err(1, "write1");

	olinep = oline;
	ibufp = eibuf = ibuf;
	v.hp = init = 0;
	pinchar = inchar;	/* XXX */
	ioff = 0;
	v.nl = -1;
	cvtime();
	frame = stk = (struct s *)setbrk(DELTA);
	dip = &d[0];
	nxf = frame + 1;
	nx = mflg;
}

/*
 * Search for file(s) in sequence:
 *	dirs given in -M (only tmac files)
 *	environment variable TERMTAB_PATH/TMAC_PATH
 *	system directories (from _PATH_TERMTAB/_PATH_TMAC)
 *	current directory.
 * ftype is type of file searched for, buf is a MAXPATHLEN buf and
 * name is the file to search for.
 * Returns name in buf if found and NULL otherwise.
 */
char *
findfile(int ftype, char *buf, char *name)
{
	char *ename = NULL;
	char *e, *pname, *path, *prepend, *postp;

	switch (ftype) {
#ifdef NROFF
	case FTERMTAB:
		ename = "TERMTAB_PATH";
		pname = _PATH_TERMTAB;
		path = fontdir;
		prepend = "tab";
		postp = ".tab";
		break;
#endif
	case FTMAC:
		ename = "TMAC_PATH";
		pname = _PATH_TMAC;
		path = Mpath;
		prepend = "tmac.";
		postp = "";
		break;
#ifndef NROFF
	case FTFONT:
		ename = "TFONT_PATH";
		pname = _PATH_TFONT;
		path = fontdir;
		prepend = "";
		postp = "";
		break;
#endif
	}

	if (path) {
		sprintf(buf, "%s/%s%s%s", path, prepend, name, postp);
		if (access(buf, R_OK) == 0)
			return buf;
	}
	if ((e = getenv(ename))) {
		/* XXX split by :? */
		sprintf(buf, "%s/%s%s%s", e, prepend, name, postp);
		if (access(buf, R_OK) == 0)
			return buf;
	}
	sprintf(buf, "%s/%s%s%s", pname, prepend, name, postp);
	if (access(buf, R_OK) == 0)
		return buf;
	strcpy(buf, name);
	if (access(buf, R_OK) == 0)
		return buf;
	return NULL;
}



cvtime()
{
	extern time_t time();
	time_t t;
	register struct tm *tmp;

	t = time((time_t *)0);
	tmp = localtime(&t);
	v.dy = tmp->tm_mday;
	v.dw = tmp->tm_wday + 1;
	v.yr = tmp->tm_year;
	v.mo = tmp->tm_mon + 1;
}
cnum(a)
char *a;
{
	register i;

	ibufp = a;
	eibuf = (char *) MAXPTR;
	i = atoix();
	ch = 0;
	return(i);
}

void
mesg(int f)
{
	static int mode;
	struct stat sbuf;

	if (ttyp==0)
		return;
	if(!f){
		stat(ttyp,&sbuf);
		mode = sbuf.st_mode;
		chmod(ttyp,mode & ~022);
	}else{
		chmod(ttyp,mode);
	}
}

int
control(a,b)
int a,b;
{
	register struct constr *cs;
	register i,j;

	i = a;
	if((i == 0) || ((cs = findmn(i)) == NULL))return(0);
	if (ISMAC(cs)) {
		nxf->nargs = 0;
		if(b)collect();
		flushi();
		return(pushi(cs->x.mx));
	}else{
		if(!b)return(0);
		(*cs->x.f)();
		return 0;
	}
}

getrq(){
	register i,j;

	if(((i=getach()) == 0) ||
	   ((j=getach()) == 0))goto rtn;
	i = PAIR(i,j);
rtn:
	return(i);
}
getch(){
	register int i, j, k;

	level++;
g0:
	if(ch){
		if(((i = ch) & CMASK) == '\n')nlflg++;
		ch = 0;
		level--;
		return(i);
	}

	if(nlflg){
		level--;
		return('\n');
	}

	if((k = (i = getch0()) & CMASK) != ESC){
		if(i & MOT)goto g2;
		if(k == FLSS){
			copyf++; raw++;
			i = getch0();
			if(!eblk.fi)flss = i;
			copyf--; raw--;
			goto g0;
		}
		if(k == RPT){
			setrpt();
			goto g0;
		}
		if(!copyf){
			if((k == 'f') && lg && !lgf){
				i = getlg(i);
				goto g2;
			}
			if((k == fc) || (k == tabch) || (k == ldrch)){
				if((i=setfield(k)) == 0)goto g0; else goto g2;
			}
			if(k == 010){
				i = makem(-width(' ' | eblk.chbits));
				goto g2;
			}
		}
		goto g2;
	}
	k = (j = getch0()) & CMASK;
	if(j & MOT){
		i = j;
		goto g2;
	}
/*
	if(k == tdelim){
		i = TDELIM;
		tdelim = IMP;
		goto g2;
	}
*/
	switch(k){

		case '\n':	/*concealed newline*/
			goto g0;
		case 'n':	/*number register*/
			setn();
			goto g0;
		case '*':	/*string indicator*/
			setstr();
			goto g0;
		case '$':	/*argument indicator*/
			seta();
			goto g0;
		case '{':	/*LEFT*/
			i = LEFT;
			goto gx;
		case '}':	/*RIGHT*/
			i = RIGHT;
			goto gx;
		case '"':	/*comment*/
			while(((i=getch0()) & CMASK ) != '\n');
			goto g2;
		case ESC:	/*double backslash*/
			i = eschar;
			goto gx;
		case 'e':	/*printable version of current eschar*/
			i = PRESC;
			goto gx;
		case ' ':	/*unpaddable space*/
			i = UNPAD;
			goto gx;
		case '|':	/*narrow space*/
			i = NARSP;
			goto gx;
		case '^':	/*half of narrow space*/
			i = HNSP;
			goto gx;
		case '\'':	/*\(aa*/
			i = 0222;
			goto gx;
		case '`':	/*\(ga*/
			i = 0223;
			goto gx;
		case '_':	/*\(ul*/
			i = 0224;
			goto gx;
		case '-':	/*current font minus*/
			i = 0210;
			goto gx;
		case '&':	/*filler*/
			i = FILLER;
			goto gx;
		case 'c':	/*to be continued*/
			i = CONT;
			goto gx;
		case ':':	/*lem's char*/
			i = COLON;
			goto gx;
		case '!':	/*transparent indicator*/
			i = XPAR;
			goto gx;
		case 't':	/*tab*/
			i = '\t';
			goto g2;
		case 'a':	/*leader (SOH)*/
			i = LEADER;
			goto g2;
		case '%':	/*ohc*/
			i = OHC;
			goto g2;
		case '.':	/*.*/
			i = '.';
		gx:
			i = (j & ~CMASK) | i;
			goto g2;
	}
	if(!copyf)
		switch(k){

			case 'p':	/*spread*/
				eblk.spread++;
				goto g0;
			case '(':	/*special char name*/
				if((i=setch()) == 0)goto g0;
				break;
			case 's':	/*size indicator*/
				setps();
				goto g0;
			case 'f':	/*font indicator*/
				setfont(0);
				goto g0;
			case 'w':	/*width function*/
				setwd();
				goto g0;
			case 'v':	/*vert mot*/
				if ((i = vmot())) break;
				goto g0;
			case 'h': 	/*horiz mot*/
				if ((i = hmot())) break;
				goto g0;
			case 'z':	/*zero with char*/
				i = setz();
				break;
			case 'l':	/*hor line*/
				setline();
				goto g0;
			case 'L':	/*vert line*/
				setvline();
				goto g0;
			case 'b':	/*bracket*/
				setbra();
				goto g0;
			case 'o':	/*overstrike*/
				setov();
				goto g0;
			case 'k':	/*mark hor place*/
				if((i=findr(getsn())) == -1)goto g0;
				vlist[i] = v.hp = sumhp();	/* XXX */
				goto g0;
			case 'j':	/*mark output hor place*/
				if(!(i=getach()))goto g0;
				i = (i<<BYTE) | JREG;
				break;
			case '0':	/*number space*/
				i = makem(width('0' | eblk.chbits));
				break;
			case 'x':	/*extra line space*/
				if ((i = xlss())) break;
				goto g0;
			case 'u':	/*half em up*/
			case 'r':	/*full em up*/
			case 'd':	/*half em down*/
				i = sethl(k);
				break;
			default:
				i = j;
		}
	else{
		ch0 = j;
		i = eschar;
	}
g2:
	if((i & CMASK) == '\n'){
		nlflg++;
		v.hp = 0;
		pinchar = inchar;	/* XXX */
		if(ipb == 0)v.cd++;
	}
	if(!--level){
		/* j = width(i); */
		/* v.hp += j; */
		/* cwidth = j; */
		if (pinchar >= inchar + LNSIZE) {	/* XXX */
			inchar[0] = makem(sumhp());
			pinchar = &inchar[1];
		}
		*pinchar++ = i;	/* XXX */
	}
	return(i);
}

sumhp()	/* XXX - add up widths in inchar array */
{
	register int n;
	register int *p;

	n = 0;
	for (p = inchar; p < pinchar; p++)
		n += width(*p);
	return(n);
}
char ifilt[32] = {0,001,002,003,0,005,006,007,010,011,012};
getch0(){
	register int i, j;

	if(ch0){i=ch0; ch0=0; return(i);}
	if(nchar){nchar--; return(rchar);}

again:
	if(cp){
		if((i = *cp++) == 0){
			cp = 0;
			goto again;
		}
	}else if(ap){
		if((i = *ap++) == 0){
			ap = 0;
			goto again;
		}
	}else if(ipb){
		if(ipb == -1)i = rdtty();
		else i = rbf(0);
	}else{
		if(donef)done(0);
		if(nx || ((ibufp >= eibuf) && (eibuf != (char *) MAXPTR))){
			if(nfo)goto g1;
		g0:
			if(nextfile()){
				if(ipb)goto again;
				if(ibufp < eibuf)goto g2;
			}
		g1:
			nx = 0;
			if((j=read(ifile,ibuf,IBUFSZ)) <= 0)goto g0;
			ibufp = ibuf;
			eibuf = ibuf + j;
			if(ipb)goto again;
		}
	g2:
		i = *ibufp++ & 0177;
		ioff++;
		if(i >= 040)goto g4; else i = ifilt[i];
	}
	if(raw)return(i);
	if((j = i & CMASK) == IMP)goto again;
	if((i == 0) && !init)goto again;
g4:
	if((copyf == 0) && ((i & ~BMASK) == 0) && ((i & CMASK) < 0370)) {
#ifndef NROFF
		if(eblk.spbits && (i>31) && ((codetab[i-32] & 0200))) i |= eblk.spbits;
		else
#endif
		i |= eblk.chbits;
	}
	if((i & CMASK) == eschar)i = (i & ~CMASK) | ESC;
	return(i);
}
nextfile(){
	register char *p;

n0:
	if(ifile)close(ifile);
	if(nx){
		p = nextf;
		if(*p != 0)goto n1;
	}
	if(ifi > 0){
		if(popf())goto n0; /*popf error*/
		return(1); /*popf ok*/
	}
	if(rargc-- <= 0)goto n2;
	p = (argp++)[0];
n1:
	if((p[0] == '-') && (p[1] == 0)){
		ifile = 0;
	}else if((ifile=open(p,0)) < 0){
		warnx("Cannot open %s", p);
		nfo -= mflg;
		done(02);
	}
	nfo++;
	v.cd = 0;
	ioff = 0;
	return(0);
n2:
	if((nfo -= mflg) && !stdi)done(0);
	nfo++;
	v.cd = ifile =  stdi = mflg = 0;
	ioff = 0;
	return(0);
}
popf(){
	register i;
	register char *p, *q;
	extern char *ttyname();

	ioff = offl[--ifi];
	ipb = iplb[ifi], ipx = iplx[ifi];
	if((ifile = ifl[ifi]) == 0){
		int ib = ipb, ix = ipx;
		ipb = xbufb, ipx = 0;
		ibufp = p = ibuf;
		while ((*p++ = rbf(1)) != 0)
			;
		eibuf = --p;
		ipb = ib, ipx = ix;
		ffree(xbufb);
		return(0);
	}
	if((lseek(ifile,(long)(ioff & ~(IBUFSZ-1)),0) < 0) ||
	   ((i = read(ifile,ibuf,IBUFSZ)) < 0))return(1);
	eibuf = ibuf + i;
	ibufp = ibuf;
	if(ttyname(ifile) == (char *)0)
		if((ibufp = ibuf + (int)(ioff & (IBUFSZ-1)))  >= eibuf)return(1);
	return(0);
}

void
flushi(){
	if(nflush)return;
	ch = 0;
	if((ch0 & CMASK) == '\n')nlflg++;
	ch0 = 0;
	copyf++;
	while(!nlflg){
		if(donef && (frame == stk))break;
		getch();
	}
	copyf--;
	v.hp = 0;
	pinchar = inchar;	/* XXX */
}
getach(){
	register i;

	lgf++;
	if(((i = getch()) & MOT) ||
	    ((i&CMASK) == ' ') ||
	    ((i&CMASK) == '\n')||
	    (i & 0200)){
			ch = i;
			i = 0;
	}
	lgf--;
	return(i & 0177);
}

void
casenx(){
	lgf++;
	skip();
	getname();
	nx++;
	nextfile();
	nlflg++;
	ipb = 0;
	ap = 0;
	nchar = eblk.pendt = 0;
	frame = stk;
	nxf = frame + 1;
}
getname(){
	register int i, j, k;

	lgf++;
	for(k=0; k < (NS-1); k++){
		if(((j=(i=getch()) & CMASK) <= ' ') ||
			(j > 0176))break;
		nextf[k] = j;
	}
	nextf[k] = 0;
	ch = i;
	lgf--;
	return(nextf[0]);
}

void
caseso(){
	register i;
	register char *p, *q;

	lgf++;
	nextf[0] = 0;
	if(skip() || !getname() || ((i=open(nextf,0)) <0) || (ifi >= NSO)) {
		warnx("can't open file %s", nextf);
		done(02);
	}
	flushi();
	ifl[ifi] = ifile;
	ifile = i;
	offl[ifi] = ioff;
	ioff = 0;
	iplb[ifi] = ipb, iplx[ifi] = ipx;
	ipb = 0;
	nx++;
	nflush++;
	if(!ifl[ifi++]){
		wbfl();
		if ((xbufb = alloc()) == 0)
			errx(1, "out of buffers");
		offsb = xbufb;
		offsx = 0;
		p = ibufp;
		while(p < eibuf)
			wbf(*p++);
		wbt(0);
	}
}

void
casecf(){	/* copy file without change */
	int fd, i, n;
	char buf[OBUFSZ];

	lgf++;
	nextf[0] = 0;
	if(skip() || !getname() || ((fd=open(nextf,0)) <0) || (ifi >= NSO)) {
		warnx("can't open file %s", nextf);
		done(02);
	}
	while ((n = read(fd, buf, OBUFSZ)) > 0)
		fwrite(buf, n, 1, fptid);
	close(fd);
}

void
getpn(a)
char *a;
{
	register i, neg;
	long atoi1();

	if((*a & 0177) == 0)return;
	neg = 0;
	ibufp = a;
	eibuf = (char *) MAXPTR;
	noscale++;
	while((i = getch() & CMASK) != 0)switch(i){
		case '+':
		case ',':
			continue;
		case '-':
			neg = MOT;
			goto d2;
		default:
			ch = i;
		d2:
			i = atoi1();
			if(nonumb)goto fini;
			else{
				*pnp++ = i | neg;
				neg = 0;
				if(pnp >= &pnlist[NPN-2]){
					warnx("Too many page numbers");
					done3(-3);
				}
			}
		}
fini:
	if(neg)*pnp++ = -2;
	*pnp = -1;
	ch = noscale = print = 0;
	pnp = pnlist;
	if(*pnp != -1)chkpn();
}
void
setrpt(){
	register i, j;

	copyf++;raw++;
	i = getch0();
	copyf--;raw--;
	if((i < 0) ||
	   (((j = getch0()) & CMASK) == RPT))return;
	rchar = j;
	nchar = i & BMASK;
}
