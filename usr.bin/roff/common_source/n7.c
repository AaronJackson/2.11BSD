#if	!defined(lint) && defined(DOSCCS)
static char sccsid[] = "@(#)n7.c	4.4 (2.11BSD) 2020/3/24";
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
#ifdef NROFF
#define GETCH gettch
#endif
#ifndef NROFF
#define GETCH getch
#endif

/*
troff7.c

text
*/

extern struct s *frame, *stk;
extern struct s *ejl;

extern int pl;
extern int trap;
extern int flss;
extern int npnflg;
extern int npn;
extern int stop;
extern int nflush;
extern int ejf;
extern int ascii;
extern int donef;
extern int dpn;
extern int ndone;
extern int pto;
extern int pfrom;
extern int print;
extern int nlist[NTRAP];
extern int mlist[NTRAP];
extern int *pnp;
extern int totout;
extern int ch;
extern int nlflg;
extern int over;
extern int nhyp;
extern int cwidth;
extern int widthp;
extern int xbitf;
extern int vflag;
extern int sfont;
extern int **hyp;
extern int nform;
extern int po;
extern int ulbit;
extern int *vlist;
extern int nrbits;
extern char trtab[];
extern int xxx;
int brflg;

static void donum(void);
static void nofill(void);
static void storeline(int, int);
void storeword(int,int);

void
tbreak(){
	register *i, j, pad;
	int res;

	trap = 0;
	if(eblk.nb)return;
	if((dip == d) && (v.nl == -1)){
		newline(1);
		return;
	}
	if(!eblk.nc){
		setnel();
		if(!eblk.wch)return;
		if(eblk.pendw)getword(1);
		movword();
	}else if(eblk.pendw && !brflg){
		getword(1);
		movword();
	}
	*eblk.linep = dip->nls = 0;
#ifdef NROFF
	if(dip == d)horiz(po);
#endif
	if(eblk.lnmod)donum();
	eblk.lastl = eblk.ne;
	if(brflg != 1){
		totout = 0;
	}else if(eblk.ad){
		if((eblk.lastl = (eblk.ll - eblk.un)) < eblk.ne)eblk.lastl = eblk.ne;
	}
	if(eblk.admod && eblk.ad && (brflg != 2)){
		eblk.lastl = eblk.ne;
		eblk.adsp = eblk.adrem = 0;
#ifdef NROFF
		if(eblk.admod == 1)eblk.un +=  quant(eblk.nel/2,t.Adj);
#endif
#ifndef NROFF
		if(eblk.admod == 1)eblk.un += eblk.nel/2;
#endif
		else if(eblk.admod ==2)eblk.un += eblk.nel;
	}
	totout++;
	brflg = 0;
	if((eblk.lastl+eblk.un) > dip->maxl)dip->maxl = (eblk.lastl+eblk.un);
	horiz(eblk.un);
#ifdef NROFF
	if(eblk.adrem%t.Adj)res = t.Hor; else res = t.Adj;
#endif
	for(i = eblk.line;eblk.nc > 0;){
		if(((j = *i++) & CMASK) == ' '){
			pad = 0;
			do{
				pad += width(j);
				eblk.nc--;
			  }while(((j = *i++) & CMASK) == ' ');
			i--;
			pad += eblk.adsp;
			--eblk.nwd;
			if(eblk.adrem){
				if(eblk.adrem < 0){
#ifdef NROFF
					pad -= res;
					eblk.adrem += res;
				}else if((totout&01) ||
					((eblk.adrem/res)>=(eblk.nwd))){
					pad += res;
					eblk.adrem -= res;
#endif
#ifndef NROFF
					pad--;
					eblk.adrem++;
				}else{
					pad++;
					eblk.adrem--;
#endif
				}
			}
			horiz(pad);
		}else{
			pchar(j);
			eblk.nc--;
		}
	}
	if(eblk.ic){
		if((j = eblk.ll - eblk.un - eblk.lastl + eblk.ics) > 0)horiz(j);
		pchar(eblk.ic);
	}
	if(eblk.icf)eblk.icf++;
		else eblk.ic = 0;
	eblk.ne = eblk.nwd = 0;
	eblk.un = eblk.in;
	setnel();
	newline(0);
	if(dip != d){if(dip->dnl > dip->hnl)dip->hnl = dip->dnl;}
	else{if(v.nl > dip->hnl)dip->hnl = v.nl;}
	for(j=eblk.ls-1; (j >0) && !trap; j--)newline(0);
	eblk.spread = 0;
}

static void
donum(){
	register i, nw;

	nrbits = eblk.nmbits;
	nw = width('1' | nrbits);
	if(eblk.nn){
		eblk.nn--;
		goto d1;
	}
	if(v.ln%eblk.ndf){
		v.ln++;
	d1:
		eblk.un += nw*(3+eblk.nms+eblk.ni);
		return;
	}
	i = 0;
	if(v.ln<100)i++;
	if(v.ln<10)i++;
	horiz(nw*(eblk.ni+i));
	nform = 0;
	fnumb(v.ln,pchar);
	eblk.un += nw*eblk.nms;
	v.ln++;
}

void
text(){
	register i;
	static int spcnt;

	nflush++;
	if((dip == d) && (v.nl == -1)){newline(1); return;}
	setnel();
	if(eblk.ce || !eblk.fi){
		nofill();
		return;
	}
	if(eblk.pendw)goto t4;
	if(eblk.pendt){ if(spcnt)goto t2; else goto t3; }
	eblk.pendt++;
	if(spcnt)goto t2;
	while(((i = GETCH()) & CMASK) == ' ')spcnt++;
	if(nlflg){
	t1:
		nflush = eblk.pendt = ch = spcnt = 0;
		callsp();
		return;
	}
	ch = i;
	if(spcnt){
	t2:
		tbreak();
		if(eblk.nc || eblk.wch)goto rtn;
		eblk.un += spcnt*eblk.sps;
		spcnt = 0;
		setnel();
		if(trap)goto rtn;
		if(nlflg)goto t1;
	}
t3:
	if(eblk.spread)goto t5;
	if(eblk.pendw || !eblk.wch)
	t4:
		if(getword(0))goto t6;
	if(!movword())goto t3;
t5:
	if(nlflg)eblk.pendt = 0;
	eblk.adsp = eblk.adrem = 0;
	if(eblk.ad){
/* jfr */	if (eblk.nwd==1) eblk.adsp=eblk.nel; else eblk.adsp=eblk.nel/(eblk.nwd-1);
#ifdef NROFF
		eblk.adsp = (eblk.adsp/t.Adj)*t.Adj;
#endif
		eblk.adrem = eblk.nel - eblk.adsp*(eblk.nwd-1);
	}
	brflg = 1;
	tbreak();
	eblk.spread = 0;
	if(!trap)goto t3;
	if(!nlflg)goto rtn;
t6:
	eblk.pendt = 0;
	ckul();
rtn:
	nflush = 0;
}

static void
nofill(){
	register i, j;

	if(!eblk.pendnf){
		over = 0;
		tbreak();
		if(trap)goto rtn;
		if(nlflg){
			ch = nflush = 0;
			callsp();
			return;
		}
		eblk.adsp = eblk.adrem = 0;
		eblk.nwd = 10000;
	}
	while((j = ((i = GETCH()) & CMASK)) != '\n'){
		if(j == eblk.ohc)continue;
		if(j == CONT){
			eblk.pendnf++;
			nflush = 0;
			flushi();
			ckul();
			return;
		}
		storeline(i,-1);
	}
	if(eblk.ce){
		eblk.ce--;
		if((i=quant(eblk.nel/2,HOR)) > 0)eblk.un += i;
	}
	if(!eblk.nc)storeline(FILLER,0);
	brflg = 2;
	tbreak();
	ckul();
rtn:
	eblk.pendnf = nflush = 0;
}
callsp(){
	register i;

	if(flss)i = flss; else i = eblk.lss;
	flss = 0;
	casespx(i);
}
ckul(){
	if(eblk.ul && (--eblk.ul == 0)){
			eblk.cu = 0;
			eblk.font = sfont;
			mchbits();
	}
	if(eblk.it && (--eblk.it == 0) && eblk.itmac)control(eblk.itmac,0);
}

static void
storeline(c,w){
	register i;

	if((c & CMASK) == JREG){
		if((i=findr(c>>BYTE)) != -1)vlist[i] = eblk.ne;
		return;
	}
	if(eblk.linep >= (eblk.line + eblk.lnsize - 1)){
		if(!over){
			warnx("Line overflow.");
			over++;
		c = 0343;
		w = -1;
		goto s1;
		}
		return;
	}
s1:
	if(w == -1)w = width(c);
	eblk.ne += w;
	eblk.nel -= w;
/*
 *	if( eblk.cu && !(c & MOT) && (trtab[(c & CMASK)] == ' '))
 *		c = ((c & ~ulbit) & ~CMASK) | '_';
 */
	*eblk.linep++ = c;
	eblk.nc++;
}

void
newline(a)
int a;
{
	register i, j, nlss;
	int opn;

	if(a)goto nl1;
	if(dip != d){
		j = eblk.lss;
		pchar1(FLSS);
		if(flss)eblk.lss = flss;
		i = eblk.lss + dip->blss;
		dip->dnl += i;
		pchar1(i);
		pchar1('\n');
		eblk.lss = j;
		dip->blss = flss = 0;
		if(dip->alss){
			pchar1(FLSS);
			pchar1(dip->alss);
			pchar1('\n');
			dip->dnl += dip->alss;
			dip->alss = 0;
		}
		if(dip->ditrap && !dip->ditf &&
			(dip->dnl >= dip->ditrap) && dip->dimac)
			if(control(dip->dimac,0)){trap++; dip->ditf++;}
		return;
	}
	j = eblk.lss;
	if(flss)eblk.lss = flss;
	nlss = dip->alss + dip->blss + eblk.lss;
	v.nl += nlss;
#ifndef NROFF
	if(ascii){dip->alss = dip->blss = 0;}
#endif
	pchar1('\n');
	flss = 0;
	eblk.lss = j;
	if(v.nl < pl)goto nl2;
nl1:
	ejf = dip->hnl = v.nl = 0;
	ejl = frame;
	if(donef == 1){
		if((!eblk.nc && !eblk.wch) || ndone)done1(0);
		ndone++;
		donef = 0;
		if(frame == stk)nflush++;
	}
	opn = v.pn;
	v.pn++;
	if(npnflg){
		v.pn = npn;
		npn = npnflg = 0;
	}
nlpn:
	if(v.pn == pfrom){
		print++;
		pfrom = -1;
	}else if(opn == pto || v.pn > pto){
		print = 0;
		opn = -1;
		chkpn();
		goto nlpn;
		}
	if(stop && print){
		dpn++;
		if(dpn >= stop){
			dpn = 0;
			dostop();
		}
	}
nl2:
	trap = 0;
	if(v.nl == 0){
		if((j = findn(0)) != NTRAP)
			trap = control(mlist[j],0);
	} else if((i = findt(v.nl-nlss)) <= nlss){
		if((j = findn1(v.nl-nlss+i)) == NTRAP){
			warnx("Trap botch.");
			done2(-5);
		}
		trap = control(mlist[j],0);
	}
}
findn1(a)
int a;
{
	register i, j;

	for(i=0; i<NTRAP; i++){
		if(mlist[i]){
			if((j = nlist[i]) < 0)j += pl;
			if(j == a)break;
		}
	}
	return(i);
}
chkpn(){
	pto = *(pnp++);
	pfrom = pto & ~MOT;
	if(pto == -1){
		done1(0);
	}
	if(pto & MOT){
		pto &= ~MOT;
		print++;
		pfrom = 0;
	}
}
findt(a)
int a;
{
	register i, j, k;

	k = 32767;
	if(dip != d){
		if(dip->dimac && ((i = dip->ditrap -a) > 0))k = i;
		return(k);
	}
	for(i=0; i<NTRAP; i++){
		if(mlist[i]){
			if((j = nlist[i]) < 0)j += pl;
			if((j -= a)  <=  0)continue;
			if(j < k)k = j;
		}
	}
	i = pl - a;
	if(k > i)k = i;
	return(k);
}
findt1(){
	register i;

	if(dip != d)i = dip->dnl;
		else i = v.nl;
	return(findt(i));
}

void
eject(struct s *a)
{
	register savlss;

	if(dip != d)return;
	ejf++;
	if(a)ejl = a;
		else ejl = frame;
	if(trap)return;
e1:
	savlss = eblk.lss;
	eblk.lss = findt(v.nl);
	newline(0);
	eblk.lss = savlss;
	if(v.nl && !trap)goto e1;
}
movword(){
	register i, w, *wp;
	int savwch, hys;

	over = 0;
	wp = eblk.wordp;
	if(!eblk.nwd){
		while(((i = *wp++) & CMASK) == ' '){
			eblk.wch--;
			eblk.wne -= width(i);
		}
		wp--;
	}
	if((eblk.wne > eblk.nel) &&
	   !eblk.hyoff && eblk.hyf &&
	   (!eblk.nwd || (eblk.nel > 3*eblk.sps)) &&
	   (!(eblk.hyf & 02) || (findt1() > eblk.lss))
	  )hyphen(wp);
	savwch = eblk.wch;
	hyp = eblk.hyptr;
	nhyp = 0;
	while(*hyp && (*hyp <= wp))hyp++;
	while(eblk.wch){
		if((eblk.hyoff != 1) && (*hyp == wp)){
			hyp++;
			if(!eblk.wdstart ||
			   ((wp > (eblk.wdstart+1)) &&
			    (wp < eblk.wdend) &&
			    (!(eblk.hyf & 04) || (wp < (eblk.wdend-1))) &&
			    (!(eblk.hyf & 010) || (wp > (eblk.wdstart+2)))
			   )
			  ){
				nhyp++;
				storeline(IMP,0);
			}
		}
		i = *wp++;
		w = width(i);
		eblk.wne -= w;
		eblk.wch--;
		storeline(i,w);
	}
	if(eblk.nel >= 0){
		eblk.nwd++;
		return(0);
	}
	xbitf = 1;
	hys = width(0200); /*hyphen*/
m1:
	if(!nhyp){
		if(!eblk.nwd)goto m3;
		if(eblk.wch == savwch)goto m4;
	}
	if(*--eblk.linep != IMP)goto m5;
	if(!(--nhyp))
		if(!eblk.nwd)goto m2;
	if(eblk.nel < hys){
		eblk.nc--;
		goto m1;
	}
m2:
	if(((i = *(eblk.linep-1) & CMASK) != '-') &&
	   (i != 0203)
	  ){
	*eblk.linep = (*(eblk.linep-1) & ~CMASK) | 0200;
	w = width(*eblk.linep);
	eblk.nel -= w;
	eblk.ne += w;
	eblk.linep++;
/*
	hsend();
*/
	}
m3:
	eblk.nwd++;
m4:
	eblk.wordp = wp;
	return(1);
m5:
	eblk.nc--;
	w = width(*eblk.linep);
	eblk.ne -= w;
	eblk.nel += w;
	eblk.wne += w;
	eblk.wch++;
	wp--;
	goto m1;
}
horiz(i)
int i;
{
	vflag = 0;
	if(i)pchar(makem(i));
}
setnel(){
	if(!eblk.nc){
		eblk.linep = eblk.line;
		if(eblk.un1 >= 0){
			eblk.un = eblk.un1;
			eblk.un1 = -1;
		}
		eblk.nel = eblk.ll - eblk.un;
		eblk.ne = eblk.adsp = eblk.adrem = 0;
	}
}
getword(x)
int x;
{
	register i, j, swp;
	int noword;

	noword = 0;
	if(x)if(eblk.pendw){
		*eblk.pendw = 0;
		goto rtn;
	}
	if ((eblk.wordp = eblk.pendw)) goto g1;
	hyp = eblk.hyptr;
	eblk.wordp = eblk.word;
	over = eblk.wne = eblk.wch = 0;
	eblk.hyoff = 0;
	while(1){
		j = (i = GETCH()) & CMASK;
		if(j == '\n'){
			eblk.wne = eblk.wch = 0;
			noword = 1;
			goto rtn;
		}
		if(j == eblk.ohc){
			eblk.hyoff = 1;
			continue;
		}
		if(j == ' '){
			storeword(i,width(i));	/* XXX */
			continue;
		}
		break;
	}
	swp = widthp;
	storeword(' ' | eblk.chbits, -1);
	if(eblk.spflg){
		storeword(' ' | eblk.chbits, -1);
		eblk.spflg = 0;
	}
	widthp = swp;
g0:
	if(j == CONT){
		eblk.pendw = eblk.wordp;
		nflush = 0;
		flushi();
		return(1);
	}
	if(eblk.hyoff != 1){
		if(j == eblk.ohc){
			eblk.hyoff = 2;
			*hyp++ = eblk.wordp;
			if(hyp > (eblk.hyptr+NHYP-1))hyp = eblk.hyptr+NHYP-1;
			goto g1;
		}
		if((j == '-') ||
		   (j == 0203) /*3/4 Em dash*/
		  )if(eblk.wordp > eblk.word+1){
			eblk.hyoff = 2;
			*hyp++ = eblk.wordp + 1;
			if(hyp > (eblk.hyptr+NHYP-1))hyp = eblk.hyptr+NHYP-1;
		}
	}
	storeword(i,width(i));	/* XXX */
g1:
	j = (i = GETCH()) & CMASK;
	if(j != ' '){
		if(j != '\n')goto g0;
		j = *(eblk.wordp-1) & CMASK;
		if((j == '.') ||
		   (j == '!') ||
		   (j == '?'))eblk.spflg++;
	}
	*eblk.wordp = 0;
rtn:
	eblk.wdstart = 0;
	eblk.wordp = eblk.word;
	eblk.pendw = 0;
	*hyp++ = 0;
	setnel();
	return(noword);
}

void
storeword(c,w)
int c, w;
{

	if(eblk.wordp >= &eblk.word[WDSIZE - 1]){
		if(!over){
			warnx("Word overflow.");
			over++;
			c = 0343;
			w = -1;
		goto s1;
		}
		return;
	}
s1:
	if(w == -1)w = width(c);
	eblk.wne += w;
	*eblk.wordp++ = c;
	eblk.wch++;
}
#ifdef NROFF
extern char trtab[];
gettch(){
	register int i, j;

	if(!((i = getch()) & MOT) && (i & ulbit)){
		j = i&CMASK;
		if(eblk.cu && (trtab[j] == ' '))
			i = ((i & ~ulbit)& ~CMASK) | '_';
		if(!eblk.cu && (j>32) && (j<0370) && !(*t.codetab[j-32] & 0200))
			i &= ~ulbit;
	}
	return(i);
}
#endif
