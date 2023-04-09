#if	!defined(lint) && defined(DOSCCS)
static char sccsid[] = "@(#)n5.c	4.3 (2.11BSD) 2020/3/24";
#endif

#include <unistd.h>
#include <stdio.h>
#include <sgtty.h>

#include "tdef.h"
extern
#include "d.h"
extern
#include "v.h"
#include "sdef.h"

/*
troff5.c

misc processing requests
*/

extern	int	inchar[LNSIZE], *pinchar;	/* XXX */
extern struct s *frame;

extern int ascii;
extern int nonumb;
extern int po;
extern int po1;
extern int nlist[NTRAP];
extern int mlist[NTRAP];
extern int lgf;
extern int pl;
extern int npn;
extern int npnflg;
extern int copyf;
extern char nextf[];
extern int trap;
extern int em;
extern int evlist[EVLSZ];
extern int evi;
extern int ibf;
extern int ev;
extern int ch;
extern int nflush;
extern int tty;
extern struct sgttyb ttys;
extern int quiet;
extern int iflg;
extern int eschar;
extern char trtab[];
extern int sfont;
extern int fontlab[];
extern int noscale;
extern int *vlist;
extern int sv;
extern int esc;
extern int nlflg;
extern int ulfont;
extern int ulbit;
extern int error;
extern int tdelim;
extern int xxx;
int iflist[NIF];
int ifx;

static void getnm(int *, int);

extern FILE *fptid;

void
casead(){
	register i;

	eblk.ad = 1;
	/*leave admod alone*/
	if(skip())return;
	switch(i = getch() & CMASK){
		case 'r':	/*right adj, left ragged*/
			eblk.admod = 2;
			break;
		case 'l':	/*left adj, right ragged*/
			eblk.admod = eblk.ad = 0;	/*same as casena*/
			break;
		case 'c':	/*centered adj*/
			eblk.admod = 1;
			break;
		case 'b': case 'n':
			eblk.admod = 0;
			break;
		case '0': case '2': case '4':
			eblk.ad = 0;
		case '1': case '3': case '5':
			eblk.admod = (i - '0')/2;
	}
}
void
casena(){
	eblk.ad = 0;
}
void
casefi(){
	tbreak();
	eblk.fi++;
	eblk.pendnf = 0;
	eblk.lnsize = LNSIZE;
}
void
casenf(){
	tbreak();
	eblk.fi = 0;
/* can't do while oline is only LNSIZE
	eblk.lnsize = LNSIZE + WDSIZE;
*/
}
void
casers(){
	dip->nls = 0;
}
void
casens(){
	dip->nls++;
}
chget(c)
int c;
{
	register i;

	if(skip() ||
	  ((i = getch()) & MOT) ||
	  ((i&CMASK) == ' ') ||
	  ((i&CMASK) == '\n')){
		ch = i;
		return(c);
	}else return(i & BMASK);
}
void
casecc(){
	eblk.cc = chget('.');
}
void
casec2(){
	eblk.c2 = chget('\'');
}
void
casehc(){
	eblk.ohc = chget(OHC);
}
void
casetc(){
	eblk.tabc = chget(0);
}
void
caselc(){
	eblk.dotc = chget(0);
}
void
casehy(){
	register i;

	eblk.hyf = 1;
	if(skip())return;
	noscale++;
	i = atoix();
	noscale = 0;
	if(nonumb)return;
	eblk.hyf = max(i,0);
}
void
casenh(){
	eblk.hyf = 0;
}
max(aa,bb)
int aa,bb;
{
	if(aa>bb)return(aa);
	else return(bb);
}
void
casece(){
	register i;

	noscale++;
	skip();
	i = max(atoix(),0);
	if(nonumb)i = 1;
	tbreak();
	eblk.ce = i;
	noscale = 0;
}
void
casein(){
	register i;

	if(skip())i = eblk.in1;
	else i = max(hnumb(&eblk.in),0);
	tbreak();
	eblk.in1 = eblk.in;
	eblk.in = i;
	if(!eblk.nc){
		eblk.un = eblk.in;
		setnel();
	}
}
void
casell(){
	register i;

	if(skip())i = eblk.ll1;
	else i = max(hnumb(&eblk.ll),INCH/10);
	eblk.ll1 = eblk.ll;
	eblk.ll = i;
	setnel();
}
void
caselt(){
	register i;

	if(skip())i = eblk.lt1;
	else i = max(hnumb(&eblk.lt),0);
	eblk.lt1 = eblk.lt;
	eblk.lt = i;
}
void
caseti(){
	register i;

	if(skip())return;
	i = max(hnumb(&eblk.in),0);
	tbreak();
	eblk.un1 = i;
	setnel();
}
void
casels(){
	register i;

	noscale++;
	if(skip())i = eblk.ls1;
	else i = max(inumb(&eblk.ls),1);
	eblk.ls1 = eblk.ls;
	eblk.ls = i;
	noscale = 0;
}
void
casepo(){
	register i;

	if(skip())i = po1;
	else i = max(hnumb(&po),0);
	po1 = po;
	po = i;
#ifndef NROFF
	if(!ascii)esc += po - po1;
#endif
}
void
casepl(){
	register i;

	skip();
	if((i = vnumb(&pl)) == 0)pl = 11 * INCH; /*11in*/
		else pl = i;
	if(v.nl > pl)v.nl = pl;
}
void
casewh(){
	register i, j, k;

	lgf++;
	skip();
	i = vnumb((int *)0);
	if(nonumb)return;
	skip();
	j = getrq();
	if((k=findn(i)) != NTRAP){
		mlist[k] = j;
		return;
	}
	for(k=0; k<NTRAP; k++)if(mlist[k] == 0)break;
	if(k == NTRAP){
		warnx("Cannot plant trap.");
		return;
	}
	mlist[k] = j;
	nlist[k] = i;
}
void
casech(){
	register i, j, k;

	lgf++;
	skip();
	if(!(j=getrq()))return;
		else for(k=0; k<NTRAP; k++)if(mlist[k] == j)break;
	if(k == NTRAP)return;
	skip();
	i = vnumb((int *)0);
	if(nonumb)mlist[k] = 0;
	nlist[k] = i;
}
findn(i)
int i;
{
	register k;

	for(k=0; k<NTRAP; k++)
		if((nlist[k] == i) && (mlist[k] != 0))break;
	return(k);
}
void
casepn(){
	register i;

	skip();
	noscale++;
	i = max(inumb(&v.pn),0);
	noscale = 0;
	if(!nonumb){
		npn = i;
		npnflg++;
	}
}
void
casebp(){
	register i;
	register struct s *savframe;

	if(dip != d)return;
	savframe = frame;
	skip();
	if((i = inumb(&v.pn)) < 0)i = 0;
	tbreak();
	if(!nonumb){
		npn = i;
		npnflg++;
	}else if(dip->nls)return;
	eject(savframe);
}
static void
casetmx(int x)
{
	register int i;
	char tmbuf[NTM];

	lgf++;
	copyf++;
	if(skip() && x)warnx("User Abort.");
	for(i=0; i<NTM-2;)if((tmbuf[i++]=getch()) == '\n')break;
	if(i == NTM-2)tmbuf[i++] = '\n';
	tmbuf[i] = 0;
	warnx(tmbuf);
	copyf--;
}

void
casespx(int a)
{
	register int i, j, savlss;

	tbreak();
	if(dip->nls || trap)return;
	i = findt1();
	if(!a){
		skip();
		j = vnumb((int *)0);
		if(nonumb)j = eblk.lss;
	}else j = a;
	if(j == 0)return;
	if(i < j)j = i;
	savlss = eblk.lss;
	if(dip != d)i = dip->dnl; else i = v.nl;
	if((i + j) < 0)j = -i;
	eblk.lss = j;
	newline(0);
	eblk.lss = savlss;
}

void
casesp(void){ casespx(0); }
void
casetm(void){ casetmx(0); }

void
casert(){
	register a, *p;

	skip();
	if(dip != d)p = &dip->dnl; else p = &v.nl;
	a = vnumb(p);
	if(nonumb)a = dip->mkline;
	if((a < 0) || (a >= *p))return;
	eblk.nb++;
	casespx(a - *p);
}
void
caseem(){
	lgf++;
	skip();
	em = getrq();
}
void
casefl(){
	tbreak();
	fflush(fptid);
}
void
caseev(){
	register nxev;

	if(skip()){
e0:
		if(evi == 0)return;
		nxev =  evlist[--evi];
		goto e1;
	}
	noscale++;
	nxev = atoix();
	noscale = 0;
	if(nonumb)goto e0;
	flushi();
	if((nxev >= NEV) || (nxev < 0) || (evi >= EVLSZ)){
		warnx("Cannot do ev.");
		if(error)done2(040);else edone(040);
		return;
	}
	evlist[evi++] = ev;
e1:
	if(ev == nxev)return;
	if (lseek(ibf, EVBASE + ev * sizeof(struct eblk), 0) < 0 ||
	    write(ibf,(char *)&eblk.block, sizeof(struct eblk)) < 0 ||
	    lseek(ibf, EVBASE + nxev*sizeof(struct eblk), 0) < 0 ||
	    read(ibf,(char *)&eblk.block, sizeof(struct eblk)) < 0)
		err(1, "envblk");
	ev = nxev;
}
void
caseel(){
	if(--ifx < 0){
		ifx = 0;
		iflist[0] = 0;
	}
	caseif(2);
}
void
caseie(){
	if(ifx >= NIF){
		warnx("if-else overflow.");
		ifx = 0;
		edone(040);
	}
	caseif(1);
	ifx++;
}
void
caseif(x)
int x;
{
	register i, notflag, true;

	if(x == 2){
		notflag = 0;
		true = iflist[ifx];
		goto i1;
	}
	true = 0;
	skip();
	if(((i = getch()) & CMASK) == '!'){
		notflag = 1;
	}else{
		notflag = 0;
		ch = i;
	}
	i = atoix();
	if(!nonumb){
		if(i > 0)true++;
		goto i1;
	}
	switch((i = getch()) & CMASK){
		case 'e':
			if(!(v.pn & 01))true++;
			break;
		case 'o':
			if(v.pn & 01)true++;
			break;
#ifdef NROFF
		case 'n':
			true++;
		case 't':
#endif
#ifndef NROFF
		case 't':
			true++;
		case 'n':
#endif
		case ' ':
			break;
		default:
			true = cmpstr(i);
	}
i1:
	true ^= notflag;
	if(x == 1)iflist[ifx] = !true;
	if(true){
	i2:
		do{
		v.hp = 0;
		pinchar = inchar;	/* XXX */
		}
		while(((i = getch()) & CMASK) == ' ');
		if((i & CMASK) == LEFT)goto i2;
		ch = i;
		nflush++;
	}else{
		copyf++;
		if(eat(LEFT) == LEFT){
			while(eatblk(RIGHT,LEFT) != RIGHT)nlflg = 0;
		}
		copyf--;
	}
}
eatblk(right,left)
int right,left;
{
	register i;

e0:
	while(((i = getch() & CMASK) != right) &&
		(i != left) &&
		(i != '\n'));
	if(i == left){
		while((i=eatblk(right,left)) != right)nlflg = 0;
		goto e0;
	}
	return(i);
}
cmpstr(delim)
int delim;
{
	register i, j;
	int bb, pb, px;
	int cnt, k;
	int savapts, savapts1, savfont, savfont1,
		savpts, savpts1;

	if(delim & MOT)return(0);
	delim &= CMASK;
	if(dip != d)wbfl();
	if ((bb = alloc()) == 0)
		return 0;
	offsb = bb;
	offsx = 0;
	cnt = 0;
	v.hp = 0;
	pinchar = inchar;	/* XXX */
	savapts = eblk.apts;
	savapts1 = eblk.apts1;
	savfont = eblk.font;
	savfont1 = eblk.font1;
	savpts = eblk.pts;
	savpts1 = eblk.pts1;
	while(((j = (i=getch()) & CMASK) != delim) && (j != '\n')){
		wbf(i);
		cnt++;
	}
	wbt(0);
	k = !cnt;
	if(nlflg)goto rtn;
	pb = bb;
	px = 0;
	eblk.apts = savapts;
	eblk.apts1 = savapts1;
	eblk.font = savfont;
	eblk.font1 = savfont1;
	eblk.pts = savpts;
	eblk.pts1 = savpts1;
	mchbits();
	v.hp = 0;
	pinchar = inchar;	/* XXX */
	while(((j = (i=getch()) & CMASK) != delim) && (j != '\n')){
		if(rbf0(pb, px) != i){
			eat(delim);
			k = 0;
			break;
		}
		if ((px = ((px+1) & (BLK-1))) == 0)
			pb = incb(pb);
		k = !(--cnt);
	}
rtn:
	eblk.apts = savapts;
	eblk.apts1 = savapts1;
	eblk.font = savfont;
	eblk.font1 = savfont1;
	eblk.pts = savpts;
	eblk.pts1 = savpts1;
	mchbits();
	offsb = dip->opb;
	offsx = dip->opx;
	ffree(bb);
	return(k);
}
void
caserd(){

	lgf++;
	skip();
	getname();
	if(!iflg){
		if(quiet){
			ttys.sg_flags &= ~ECHO;
			stty(0, &ttys);
			fputc('', fptid); /*bell*/
		}else{
			if(nextf[0]){
				fprintf(stderr, "%s:", nextf);
			}else{
				fputc('', fptid); /*bell*/
			}
		}
	}
	collect();
	tty++;
	pushi(-1);
}
rdtty(){
	char onechar;

	onechar = 0;
	if(read(0, &onechar, 1) == 1){
		if(onechar == '\n')tty++;
			else tty = 1;
		if(tty != 3)return(onechar);
	}
	popi();
	tty = 0;
	if(quiet){
		ttys.sg_flags |= ECHO;
		stty(0, &ttys);
	}
	return(0);
}
void
caseec(){
	eschar = chget('\\');
}
void
caseeo(){
	eschar = 0;
}

void
caseta(){
	register i;

	eblk.tabtab[0] = nonumb = 0;
	for(i=0; ((i < (NTAB-1)) && !nonumb); i++){
		if(skip())break;
		eblk.tabtab[i] = eblk.tabtab[max(i-1,0)] & TMASK;
		eblk.tabtab[i] = max(hnumb(&eblk.tabtab[i]),0) & TMASK;
		if(!nonumb) switch(ch & CMASK){
			case 'C':
				eblk.tabtab[i] |= CTAB;
				break;
			case 'R':
				eblk.tabtab[i] |= RTAB;
				break;
			default: /*includes L*/
				break;
			}
		nonumb = ch = 0;
	}
	eblk.tabtab[i] = 0;
}
void
casene(){
	register i, j;

	skip();
	i = vnumb((int *)0);
	if(nonumb)i = eblk.lss;
	if(i > (j = findt1())){
		i = eblk.lss;
		eblk.lss = j;
		dip->nls = 0;
		newline(0);
		eblk.lss = i;
	}
}
void
casetr(){
	register i, j;

	lgf++;
	skip();
	while((i = getch() & CMASK) != '\n'){
		if((i & MOT) || ((j = getch()) & MOT))return;
		if((j &= CMASK) == '\n')j = ' ';
		trtab[i] = j;
	}
}
void
casecu(){
	eblk.cu++;
	caseul();
}
void
caseul(){
	register i;

	noscale++;
	if(skip())i = 1;
	else i = atoix();
	if(eblk.ul && (i == 0)){
		eblk.font = sfont;
		eblk.ul = eblk.cu = 0;
	}
	if(i){
		if(!eblk.ul){
			sfont = eblk.font;
			eblk.font = ulfont;
		}
		eblk.ul = i;
	}
	noscale = 0;
	mchbits();
}
void
caseuf(){
	register i, j;

	if(skip() || !(i = getrq()) || (i == 'S') ||
		((j = find(i,fontlab))  == -1))
			ulfont = 1; /*default position 2*/
	else ulfont = j;
#ifdef NROFF
	if(ulfont == 0)ulfont = 1;
#endif
	ulbit = ulfont<<9;
}
void
caseit(){
	register i;

	lgf++;
	eblk.it = eblk.itmac = 0;
	noscale++;
	skip();
	i = atoix();
	skip();
	if(!nonumb && (eblk.itmac = getrq()))eblk.it = i;
	noscale = 0;
}
void
casemc(){
	register i;

	if(eblk.icf > 1)eblk.ic = 0;
	eblk.icf = 0;
	if(skip())return;
	eblk.ic = getch();
	eblk.icf = 1;
	skip();
	i = max(hnumb((int *)0),0);
	if(!nonumb)eblk.ics = i;
}
void
casemk(){
	register i, j;

	if(dip != d)j = dip->dnl; else j = v.nl;
	if(skip()){
		dip->mkline = j;
		return;
	}
	if((i = getrq()) == 0)return;
	vlist[findr(i)] = j;
}
void
casesv(){
	register i;

	skip();
	if((i = vnumb((int *)0)) < 0)return;
	if(nonumb)i = 1;
	sv += i;
	caseos();
}
void
caseos(){
	register savlss;

	if(sv <= findt1()){
		savlss = eblk.lss;
		eblk.lss = sv;
		newline(0);
		eblk.lss = savlss;
		sv = 0;
	}
}
void
casenm(){
	register i;

	eblk.lnmod = eblk.nn = 0;
	if(skip())return;
	eblk.lnmod++;
	noscale++;
	i = inumb(&v.ln);
	if(!nonumb)v.ln = max(i,0);
	getnm(&eblk.ndf,1);
	getnm(&eblk.nms,0);
	getnm(&eblk.ni,0);
	noscale = 0;
	eblk.nmbits = eblk.chbits;
}

static void
getnm(p,min)
int *p, min;
{
	register i;

	eat(' ');
	if(skip())return;
	i = atoix();
	if(nonumb)return;
	*p = max(i,min);
}
void
casenn(){
	noscale++;
	skip();
	eblk.nn = max(atoix(),1);
	noscale = 0;
}
void
caseab(){
	dummy();
	casetmx(1);
	done2(0);
}
