#if	!defined(lint) && defined(DOSCCS)
static char sccsid[] = "@(#)n3.c	4.5 (2.11BSD) 2020/3/24";
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "tdef.h"
extern
#include "d.h"
extern
#include "v.h"
#ifdef NROFF
#include "tw.h"
#endif
#include "sdef.h"

/*
troff3.c

macro and string routines, storage allocation
*/

extern struct s *frame, *stk, *nxf;
extern char *enda;

extern int ch;
extern int ibf;
extern int lgf;
extern int copyf;
extern int ch0;
extern int nlflg;
extern int *argtop;
extern int *ap;
extern int nchar;
extern int rchar;
extern int dilev;
extern int nonumb;
extern int nrbits;
extern int nform;
extern int fmt[];
extern int macerr;
extern int wbfi;
extern int po;
extern int *cp;
extern int xxx;
int pagech = '%';
int strflg;

int wbuf[BLK];
int rbufx[BLKCSZ][BLK];
static int curoff[BLKCSZ];
int woffb;
int nextbb, nextbx;
int offsb, offsx;

static int blist[NBLIST];
static char mbuf[MAXSTKSZ * sizeof(int)];
static int mbufp;
static int lastblk = 1;
static int woffx;
static int apptrb, apptrx;
static struct constr *conhsh[CONHSZ];
static struct constr *oldmnp, *newmnp;

static int finds(int mn, int isapp);
static void clrmn(register struct constr *cs);
static void commdef(int flags);
static void dodi(int isapp);

#define	ISAPP	1
#define	ISSTR	2
#define	ISDIV	4

#if 0
#define	HSHFTN(h, s) for (h = *s++; *s; h += *s++); h %= CONHSZ
#else
#define HSHFTN(s) (((s & 0377) + (s >> 8)) % CONHSZ)
#endif
/*
 * Insert entry cs into hash table.
 */
static void
inshsh(register struct constr *cs)
{
	register int h;

	h = HSHFTN(cs->rq);
	cs->next = conhsh[h];
	conhsh[h] = cs;
}

/*
 * Insert all requests into the hash table.
 */
void
inithsh(void)
{
	register struct constr *cs;

	for (cs = &constr[0]; cs->rq; cs++)
		inshsh(cs);
}

/*
 * Find an entry in the hash table.
 * return the entry or NULL.
 */
struct constr *
findhsh(char *str)
{
	register struct constr *cs;
	register int h, ls;

	ls = PAIR(str[0], str[1]);
	h = HSHFTN(ls);
	for (cs = conhsh[h]; cs; cs = cs->next)
		if (ls == cs->rq)
			return cs;
	return NULL;
}

/*
 * remove entry from the hash table.
 * return the entry or NULL if not found.
 */
void
remhsh(register struct constr *rcs)
{
	register struct constr *cs;
	register int h;

	h = HSHFTN(rcs->rq);
	if (rcs == conhsh[h])
		conhsh[h] = rcs->next;
	else for (cs = conhsh[h]; cs; cs = cs->next) {
		if (rcs != cs->next)
			continue;
		cs->next = rcs->next;
	}
}

void
caseig(){
	register i;

	offsb = 0;
	if((i = copyb()) != '.')control(i,1);
}
void
casern(){
	register i,j;
	register struct constr *fcs, *cs;

	lgf++;
	skip();
	if(((i=getrq())==0) || ((fcs = findmn(i)) == NULL))
		return; /* no "from" name */
	skip();
	cs = findmn(j=getrq());
	if (cs) clrmn(cs);

	(void)remhsh(fcs);
	fcs->rq = j;
	inshsh(fcs);
}
void
caserm(){
	register struct constr *cs;

	lgf++;
	while(!skip()){
		cs = findmn(getrq());
		if (cs) clrmn(cs);
	}
}

void
caseas()
{
	commdef(ISAPP|ISSTR);
}

void
caseds(){
	commdef(ISSTR);
}

void
caseam()
{
	commdef(ISAPP);
}

void
casede()
{
	commdef(0);
}

static void
commdef(int flags)
{
	register i, req;
	int sb, sx;

	if(dip != d)wbfl();
	req = '.';
	lgf++;
	skip();
	if((i=getrq())==0)
		return;
	if (finds(i, flags & ISAPP) == 0)
		return;
	if(flags & ISSTR)
		copys();
	else
		req = copyb();

	wbfl();
	clrmn(oldmnp);
	if (newmnp) {
		if (newmnp->rq)
			remhsh(newmnp);
		newmnp->rq = i;
		newmnp->flags |= CONMAC;
		inshsh(newmnp);
	}
	if(apptrb){
		sb = offsb, sx = offsx;
		offsb = apptrb, offsx = apptrx;
		wbt(IMP);
		offsb = sb, offsx = sx;
	}
	offsb = dip->opb;
	offsx = dip->opx;
	if(req != '.')control(req,1);
}

static struct constr *mnpole;

static struct constr *
getmn(char *n)
{
	register int i;
	register struct constr *cs, *ocs;

	if (mnpole == NULL) {
		ocs = mnpole;
		if ((mnpole = malloc(sizeof(struct constr) * MACHUNK)) == NULL)
			err(1, "getmn");
		for (i = 0; i < MACHUNK-1; i++)
			mnpole[i].next = &mnpole[i+1];
		mnpole[MACHUNK-1].next = ocs;
	}
	cs = mnpole, mnpole = cs->next;

	cs->rq = PAIR(n[0], n[1]);
	cs->flags = CONMAC;
	cs->x.mx = 0;
	cs->next = NULL;
	return cs;
}

struct constr *
findmn(i)
int i;
{
	register j;
	register struct constr *cs;
	char c[3];

	c[0] = i; c[1] = i >> 8; c[2] = 0;
	cs = findhsh(c);
	return cs;
}

void
clrmn(register struct constr *cs)
{
	if (cs == NULL)
		return;
	if(ISMAC(cs))
		ffree(cs->x.mx);
	if (cs->rq)
		remhsh(cs);
	cs->next = mnpole, mnpole = cs;
}

/*
 * 
 */
static int
finds(int mn, int flags)
{
	register i;
	register struct constr *cs;
	register int savipb, savipx;
	char c3[3];

	oldmnp = cs = findmn(mn);
	newmnp = NULL;
	apptrb = 0;
	if((flags & ISAPP) && cs && ISMAC(cs)){
		savipb = ipb, savipx = ipx;
		ipb = cs->x.mx, ipx = 0;
		oldmnp = NULL;
		while((i=rbf(flags & ISAPP)) != 0);
		apptrb = ipb, apptrx = ipx;
		if ((flags & ISDIV) == 0) {
			if ((ipx = ((ipx+1) & (BLK-1))) == 0)
				ipb = incb(ipb);
		}
		nextbb = ipb;
		nextbx = ipx;
		ipb = savipb, ipx = savipx;
	}else{
		c3[0] = mn, c3[1] = mn >> 8, c3[2] = 0;
		cs = getmn(c3);
		if ((cs->x.mx = alloc()) == 0) {
			if(macerr++ > 1)done2(02);
			warnx("Too many string/macro names.");
			edone(04);
			offsb = 0;
			return 0;
		}
		if ((flags & ISDIV) == 0) {
			newmnp = cs;
			if(oldmnp == NULL)
				cs->rq = -1;
		}else{
			inshsh(cs);
		}
	}

	offsb = nextbb, offsx = nextbx;
	return (offsb);
}
skip(){
	register i;

	while(((i=getch()) & CMASK) == ' ');
	ch=i;
	return(nlflg);
}
copyb()
{
	register i, j, k;
	int ii, req, state;
	int sb, sx;

	if(skip() || !(j=getrq()))j = '.';
	req = j;
	k = j>>BYTE;
	j &= BMASK;
	copyf++;
	flushi();
	nlflg = 0;
	state = 1;
	while(1){
		i = (ii = getch()) & CMASK;
		if(state == 3){
			if(i == k)break;
			if(!k){
				ch = ii;
				i = getach();
				ch = ii;
				if(!i)break;
			}
			state = 0;
			goto c0;
		}
		if(i == '\n'){
			state = 1;
			nlflg = 0;
			goto c0;
		}
		if((state == 1) && (i == '.')){
			state++;
			sb = offsb, sx = offsx;
			goto c0;
		}
		if((state == 2) && (i == j)){
			state++;
			goto c0;
		}
		state = 0;
c0:
		if(offsb)wbf(ii);
	}
	if(offsb){
		wbfl();
		offsb = sb, offsx = sx;
		wbt(0);
	}
	copyf--;
	return(req);
}
copys()
{
	register i;

	copyf++;
	if(skip())goto c0;
	if(((i=getch()) & CMASK) != '"')wbf(i);
	while(((i=getch()) & CMASK) != '\n')wbf(i);
c0:
	wbt(0);
	copyf--;
}
/*
 * get next free entry in the temp file.
 * Returns index on success, otherwise 0.
 * sets nextb to resulting offset.
 * XXX optimize search.
 */
int
alloc(void)
{
	register int i;

	for (i = lastblk; i < NBLIST && blist[i]; i++)
		;
	
	if (i == NBLIST) {
		i = 0;
	} else {
		blist[i] = -1;
		lastblk = i + 1;
	}
	nextbb = i, nextbx = 0;

	return i;
}

/*
 * Free a block chain.
 */
void
ffree(register int j)
{
	register int k;

	while ((k = blist[j]) != -1) {
		if (j < lastblk)
			lastblk = j;
		blist[j] = 0;
		j = k;
	}
	if (j < lastblk)
		lastblk = j;
	blist[j] = 0;
}
wbt(i)
int i;
{
	wbf(i);
	wbfl();
}

/*
 * Write a tchar to temp file at position (offsb, offsx).
 * Increment (offsb, offsx).  Flush and allocate more buffers if needed.
 */
void
wbf(i)
int i;
{
	register int j, k;

	if(!offsb)return;
	if(!woffb){
		woffb = offsb;
		woffx = offsx;
		wbfi = 0;
	}
	/*
	 * Write to buffer at position wbfi.
	 * Position in file is (offsb, offsx).
	 * (woffb, woffx) is the start of writing to buffer.
	 */
	wbuf[wbfi++] = i;
	if ((offsx = ((offsx+1) & (BLK-1))) == 0) {
		wbfl();
		j = offsb;
		if((k = blist[j]) == -1){
			if((k = alloc()) == 0){
				warnx("Out of temp file space.");
				done2(01);
			}
			blist[j] = k;
		}
		offsb = k;
	}
	if(wbfi >= BLK)wbfl();
}
/*
 * Write (part of) buffer from pos (woffb, woffx) and wbfi words.
 */
void
wbfl(void)
{
	register int lb;

	if (woffb == 0)
		return;

	lseek(ibf, (((long)woffb << BLKBITS) | woffx) * sizeof(int), 0);
	write(ibf, (char *)wbuf, wbfi * sizeof(int));

	lb = woffb & (BLKCSZ-1);
	if (woffb == curoff[lb])
		curoff[lb] = 0;
	woffb = 0;
}

/*
 * Get next char from buffer and increment position (if not end of buffer).
 * If not appending pop stack if end of buffer.
 */
int
rbf(int isapp)
{
	register i, j;

	if (ipb == curoff[j = ipb & (BLKCSZ-1)] && (i = rbufx[j][ipx]))
		goto qdone;
		
	if((i=rbf0(ipb, ipx)) == 0){
		if (!isapp)
			i = popi();
	}else{
qdone:		if ((ipx = ((ipx+1) & (BLK-1))) == 0)
			ipb = incb(ipb);
	}
	return(i);
}
/*
 * Get next char from position (buf, off).
 */
int
rbf0(register int b, register int x)
{
	register int lb = b & (BLKCSZ-1);

	if (b != curoff[lb]) {
		curoff[lb] = b;
		lseek(ibf, ((long)curoff[lb] << BLKBITS) * sizeof(int), 0);
		if(read(ibf, (char *)rbufx[lb], BLK * sizeof(int)) == 0)return(0);
	}
	return(rbufx[lb][x]);
}

/*
 * Traverse to next buffer. Return new buffer index.
 */
int
incb(register int b)
{
	if ((b = blist[b]) == -1) {
		warnx("Bad storage allocation.");
		done2(-5);
	}
	return b;
}

popi(){
	register struct s *p;

	if(frame == stk)return(0);
	if(strflg)strflg--;
	p = nxf = frame;
	p->nargs = 0;
	frame = p->pframe;
	ipb = p->pipb;
	ipx = p->pipx;
	nchar = p->pnchar;
	rchar = p->prchar;
	eblk.pendt = p->ppendt;
	ap = p->pap;
	cp = p->pcp;
	ch0 = p->pch0;
	return(p->pch);
}

/*
 *	test that the end of the allocation is above a certain location
 *	in memory
 */
#define SPACETEST(base, size) while ((&mbuf[mbufp] - (size)) <= (char *)(base)){setbrk(DELTA);}

int
pushi(int bufoff)
{
	register struct s *p;

	SPACETEST(nxf, sizeof(struct s));
	p = nxf;
	p->pframe = frame;
	p->pipx = ipx;
	p->pipb = ipb;
	p->pnchar = nchar;
	p->prchar = rchar;
	p->ppendt = eblk.pendt;
	p->pap = ap;
	p->pcp = cp;
	p->pch0 = ch0;
	p->pch = ch;
	cp = ap = 0;
	nchar = rchar = eblk.pendt = ch0 = ch = 0;
	frame = nxf;
	if (nxf->nargs == 0) 
		nxf += 1;
	else 
		nxf = (struct s *)argtop;
	ipb = bufoff, ipx = 0;
	return bufoff;
}

char *
setbrk(int x)
{
	register char *i;

	x += sizeof(int) - 1;
	x &= ~(sizeof(int) - 1);
	if (x + mbufp >= MAXSTKSZ * sizeof(int)) {
		warnx("Core limit reached.");
		edone(0100);
	}
	i = &mbuf[mbufp];
	mbufp += x;
	return(i);
}

getsn()
{
	register i;

	if ((i = getach()) == 0)
		return(0);
	if (i == '(')
		return(getrq());
	else 
		return(i);
}


setstr()
{
	register struct constr *cs;
	register i;

	lgf++;
	if (    ((i = getsn()) == 0)
	     || ((cs = findmn(i)) == NULL)
	     ||  !(ISMAC(cs))) {
		lgf--;
		return(0);
	} else {
		SPACETEST(nxf, sizeof(struct s));
		nxf->nargs = 0;
		strflg++;
		lgf--;
		return(pushi(cs->x.mx));
	}
}

typedef	int	tchar;
#define	cbits(x)	((x) & CMASK)

collect()
{
	register j;
	tchar i;
	register tchar *strp;
	tchar * lim;
	tchar * *argpp, **argppend;
	int	quote;
	struct s *savnxf;

	copyf++;
	nxf->nargs = 0;
	savnxf = nxf;
	if (skip())
		goto rtn;

	{
		char *memp;
		memp = (char *)savnxf;
		/*
		 *	1 s structure for the macro descriptor
		 *	APERMAC tchar *'s for pointers into the strings
		 *	space for the tchar's themselves
		 */
		memp += sizeof(struct s);
		/*
		 *	CPERMAC (the total # of characters for ALL arguments)
		 *	to a macros, has been carefully chosen
		 *	so that the distance between stack frames is < DELTA 
		 */
#define	CPERMAC	200
#define	APERMAC	9
		memp += APERMAC * sizeof(tchar *);
		memp += CPERMAC * sizeof(tchar);
		nxf = (struct s*)memp;
	}
	lim = (tchar *)nxf;
	argpp = (tchar **)(savnxf + 1);
	argppend = &argpp[APERMAC];
	SPACETEST(argppend, sizeof(tchar *));
	strp = (tchar *)argppend;
	/*
	 *	Zero out all the string pointers before filling them in.
	 */
	for (j = 0; j < APERMAC; j++){
		argpp[j] = (tchar *)0;
	}
#if 0
	fprintf(stderr, "savnxf=0x%x,nxf=0x%x,argpp=0x%x,strp=argppend=0x%x,lim=0x%x,enda=0x%x\n",
		savnxf, nxf, argpp, strp, lim, enda);
#endif
	strflg = 0;
	while ((argpp != argppend) && (!skip())) {
		*argpp++ = strp;
		quote = 0;
		if (cbits(i = getch()) == '"')
			quote++;
		else 
			ch = i;
		while (1) {
			i = getch();
			if ( nlflg ||  (!quote && cbits(i) == ' '))
				break;
			if (   quote
			    && (cbits(i) == '"')
			    && (cbits(i = getch()) != '"')) {
				ch = i;
				break;
			}
			*strp++ = i;
			if (strflg && (strp >= lim)) {
#if 0
				fprintf(stderr, "strp=0x%x, lim = 0x%x\n",
					strp, lim);
#endif
				warnx("Macro argument too long.");
				copyf--;
				edone(004);
			}
			SPACETEST(strp, 3 * sizeof(tchar));
		}
		*strp++ = 0;
	}
	nxf = savnxf;
	nxf->nargs = argpp - (tchar **)(savnxf + 1);
	argtop = strp;
rtn:
	copyf--;
}


seta()
{
	register i;

	if(((i = (getch() & CMASK) - '0') > 0) &&
		(i <= APERMAC) && (i <= frame->nargs))ap = *((int **)frame + i-1 + (sizeof(struct s)/sizeof(int **)));
}

void
caseda()
{
	dodi(ISAPP|ISDIV);
}

void
casedi()
{
	dodi(ISDIV);
}

static void
dodi(int flags)
{
	register i, j;
	register *k;

	lgf++;
	if(skip() || ((i=getrq()) == 0)){
		if(dip != d)wbt(0);
		if(dilev > 0){
			v.dn = dip->dnl;
			v.dl = dip->maxl;
			dip = &d[--dilev];
			offsb = dip->opb;
			offsx = dip->opx;
		}
		return;
	}
	if(++dilev == NDI){
		--dilev;
		warnx("Cannot divert.");
		edone(02);
	}
	if(dip != d)wbt(0);
	dip = &d[dilev];
	finds(i, flags);
	dip->opb = offsb;
	dip->opx = offsx;
	dip->curd = i;
	clrmn(oldmnp);
	k = (int *)&dip->dnl;
	for(j=0; j<10; j++)k[j] = 0;	/*not op and curd*/
}

void
casedt(void)
{
	lgf++;
	dip->dimac = dip->ditrap = dip->ditf = 0;
	skip();
	dip->ditrap = vnumb((int *)0);
	if(nonumb)return;
	skip();
	dip->dimac = getrq();
}

/*
 * Helper due to code design below.
 */
static int
xpchar(int num)
{
	pchar(num);
	return 0;
}

void
casetl(){
	register i, bb;
	int w1, w2, w3, delim;
	extern width();

	dip->nls = 0;
	skip();
	if(dip != d)wbfl();
	if ((bb = alloc()) == 0)
		return;
	offsb = bb;
	offsx = 0;
	if((delim = getch()) & MOT){
		ch = delim;
		delim = '\'';
	}else delim &= CMASK;
	if(!nlflg)
		while(((i = getch()) & CMASK) != '\n'){
			if((i & CMASK) == delim)i = IMP;
			wbf(i);
		}
	wbf(IMP);wbf(IMP);wbt(0);

	w1 = hseg(width, bb, 0);
	w2 = hseg(width, 0, 0);
	w3 = hseg(width, 0, 0);
	offsb = dip->opb;
	offsx = dip->opx;
#ifdef NROFF
	if(!offsb)horiz(po);
#endif
	hseg(xpchar, bb, 0);
	if(w2 || w3)horiz(i=quant((eblk.lt - w2)/2-w1,HOR));
	hseg(xpchar, 0, 0);
	if(w3){
		horiz(eblk.lt-w1-w2-w3-i);
		hseg(xpchar, 0, 0);
	}
	newline(0);
	if(dip != d){if(dip->dnl > dip->hnl)dip->hnl = dip->dnl;}
	else{if(v.nl > dip->hnl)dip->hnl = v.nl;}
	ffree(bb);
}
void
casepc(){
	pagech = chget(IMP);
}

int
hseg(int (*f)(),int pb, int px)
{
	register acc, i;
	static int qb, qx;

	acc = 0;
	if(pb){ qb = pb, qx = px; }
	while(1){
		i = rbf0(qb, qx);
		if ((qx = ((qx+1) & (BLK-1))) == 0)
			qb = incb(qb);
		if(!i || (i == IMP))return(acc);
		if((i & CMASK) == pagech){
			nrbits = i & ~CMASK;
			nform = fmt[findr('%')];
			acc += fnumb(v.pn,f);
		}else acc += (*f)(i);
	}
}
void
casepm(){
	register i, k;
	register char *p;
	register struct constr *cs;
	int xx, cnt, kk, tot;
	int j;
	char *kvt();
	char pmline[10];

	kk = cnt = 0;
	tot = !skip();
	for (i = 0; i < CONHSZ; i++) {
	    for (cs = conhsh[i]; cs; cs = cs->next) {
		if (!ISMAC(cs))
			continue;
		xx = cs->rq;
		p = pmline;
		j = cs->x.mx;
		k = 1;
		while((j = blist[j]) != -1)k++;
		cnt++;
		kk += k;
		if(!tot){
			*p++ = xx & 0177;
			if(!(*p++ = (xx >> BYTE) & 0177))*(p-1) = ' ';
			*p++ = ' ';
			kvt(k,p);
			fprintf(stderr, "%s", pmline);
		}
	    }
	}
	if(tot || (cnt > 1)){
		kvt(kk,pmline);
		fprintf(stderr, "%s", pmline);
	}
}
char *kvt(k,p)
int k;
char *p;
{
	if(k>=100)*p++ = k/100 + '0';
	if(k>=10)*p++ = (k%100)/10 + '0';
	*p++ = k%10 + '0';
	*p++ = '\n';
	*p = 0;
	return(p);
}
dummy(){}
