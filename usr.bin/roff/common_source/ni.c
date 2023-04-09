#if	!defined(lint) && defined(DOSCCS)
static char sccsid[] = "@(#)ni.c  4.3  (2.11BSD) 2020/3/24";
#endif

#include "tdef.h"

int r[NN] = {
	PAIR('%',0),
	PAIR('n','l'),
	PAIR('y','r'),
	PAIR('h','p'),
	PAIR('c','t'),
	PAIR('d','n'),
	PAIR('m','o'),
	PAIR('d','y'),
	PAIR('d','w'),
	PAIR('l','n'),
	PAIR('d','l'),
	PAIR('s','t'),
	PAIR('s','b'),
	PAIR('c','.')};
int pto = 10000;
int pfrom = 1;
int print = 1;
char nextf[NS] = "/usr/share/tmac/tmac.";
#ifndef NROFF
int oldbits = -1;
#endif
int init = 1;
int fc = IMP;
int eschar = '\\';
int pl = 11*INCH;
int po = PO;
int dfact = 1;
int dfactd = 1;
int res = 1;
int smnt = 4;
int ascii = ASCII;
char ptname[] = "/dev/cat";
int lg = LG;
int pnlist[NPN] = {-1};
int *pnp = pnlist;
int npn = 1;
int npnflg = 1;
int xflg = 1;
int dpn = -1;
int totout = 1;
int ulfont = 1;
int ulbit = 1<<9;
int tabch = TAB;
int ldrch = LEADER;
int xxx;

#define	CONENT(x,y,z) 	{ 0, z, PAIR(x, y), 0 }
struct constr constr[] = {
	CONENT('d','s',caseds),
	CONENT('a','s',caseas),
	CONENT('s','p',casesp),
	CONENT('f','t',caseft),
	CONENT('p','s',caseps),
	CONENT('v','s',casevs),
	CONENT('n','r',casenr),
	CONENT('i','f',caseif),
	CONENT('i','e',caseie),
	CONENT('e','l',caseel),
	CONENT('p','o',casepo),
	CONENT('t','l',casetl),
	CONENT('t','m',casetm),
	CONENT('b','p',casebp),
	CONENT('c','h',casech),
	CONENT('p','n',casepn),
	CONENT('b','r',tbreak),
	CONENT('t','i',caseti),
	CONENT('n','e',casene),
	CONENT('n','f',casenf),
	CONENT('c','e',casece),
	CONENT('f','i',casefi),
	CONENT('i','n',casein),
	CONENT('l','l',casell),
	CONENT('n','s',casens),
	CONENT('m','k',casemk),
	CONENT('r','t',casert),
	CONENT('a','m',caseam),
	CONENT('d','e',casede),
	CONENT('d','i',casedi),
	CONENT('d','a',caseda),
	CONENT('w','h',casewh),
	CONENT('d','t',casedt),
	CONENT('i','t',caseit),
	CONENT('r','m',caserm),
	CONENT('r','r',caserr),
	CONENT('r','n',casern),
	CONENT('a','d',casead),
	CONENT('r','s',casers),
	CONENT('n','a',casena),
	CONENT('p','l',casepl),
	CONENT('t','a',caseta),
	CONENT('t','r',casetr),
	CONENT('u','l',caseul),
	CONENT('c','u',casecu),
	CONENT('l','t',caselt),
	CONENT('n','x',casenx),
	CONENT('s','o',caseso),
	CONENT('i','g',caseig),
	CONENT('t','c',casetc),
	CONENT('f','c',casefc),
	CONENT('e','c',caseec),
	CONENT('e','o',caseeo),
	CONENT('l','c',caselc),
	CONENT('e','v',caseev),
	CONENT('r','d',caserd),
	CONENT('a','b',caseab),
	CONENT('f','l',casefl),
	CONENT('e','x',done),
	CONENT('s','s',casess),
	CONENT('f','p',casefp),
	CONENT('c','s',casecs),
	CONENT('b','d',casebd),
	CONENT('l','g',caselg),
	CONENT('h','c',casehc),
	CONENT('h','y',casehy),
	CONENT('n','h',casenh),
	CONENT('n','m',casenm),
	CONENT('n','n',casenn),
	CONENT('s','v',casesv),
	CONENT('o','s',caseos),
	CONENT('l','s',casels),
	CONENT('c','c',casecc),
	CONENT('c','2',casec2),
	CONENT('e','m',caseem),
	CONENT('a','f',caseaf),
	CONENT('h','w',casehw),
	CONENT('m','c',casemc),
	CONENT('p','m',casepm),
#ifdef NROFF
	CONENT('p','i',casepi),
#endif
	CONENT('u','f',caseuf),
	CONENT('p','c',casepc),
	CONENT('h','t',caseht),
#ifndef NROFF
	CONENT('f','z',casefz),
#endif
	CONENT('c', 'f',casecf),
	CONENT(0, 0, 0),
};

/*
troff environment block
*/

struct eblk eblk = {
	0, ICS, 0, 0, 0, 0, 0, PS,
	PS, PS, PS, FT, FT, SPS, SS, VS,
	VS, 1, 1, LL, LL, LL, LL, 1,
	1, 1, 1, '.', '\'', OHC, IMP, 1,
	0, -1, 0, '.', 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, LNSIZE,
	{ 0 },
	{DTAB,DTAB*2,DTAB*3,DTAB*4,DTAB*5,DTAB*6,DTAB*7,DTAB*8,
	    DTAB*9,DTAB*10,DTAB*11,DTAB*12,DTAB*13,DTAB*14,DTAB*15,0},
	{ 0 },
	{ 0 },
};

#if 0
int block = 0;
int ics = ICS;
int ic = 0;
int icf = 0;
int chbits = 0;
int spbits = 0;
int nmbits = 0;
int apts = PS;
int apts1 = PS;
int pts = PS;
int pts1 = PS;
int font = FT;
int font1 = FT;
int sps = SPS;
int spacesz = SS;
int lss = VS;
int lss1 = VS;
int ls = 1;
int ls1 = 1;
int ll = LL;
int ll1 = LL;
int lt = LL;
int lt1 = LL;
int ad = 1;
int nms = 1;
int ndf = 1;
int fi = 1;
int cc = '.';
int c2 = '\'';
int ohc = OHC;
int tdelim = IMP;
int hyf = 1;
int hyoff = 0;
int un1 = -1;
int tabc = 0;
int dotc = '.';
int adsp = 0;
int adrem = 0;
int lastl = 0;
int nel = 0;
int admod = 0;
int *wordp = 0;
int spflg = 0;
int *linep = 0;
int *wdend = 0;
int *wdstart = 0;
int wne = 0;
int ne = 0;
int nc = 0;
int nb = 0;
int lnmod = 0;
int nwd = 0;
int nn = 0;
int ni = 0;
int ul = 0;
int cu = 0;
int ce = 0;
int in = 0;
int in1 = 0;
int un = 0;
int wch = 0;
int pendt = 0;
int *pendw = 0;
int pendnf = 0;
int spread = 0;
int it = 0;
int itmac = 0;
int lnsize = LNSIZE;
int *hyptr[NHYP] = {0};
int tabtab[NTAB] = {DTAB,DTAB*2,DTAB*3,DTAB*4,DTAB*5,DTAB*6,DTAB*7,DTAB*8,
	DTAB*9,DTAB*10,DTAB*11,DTAB*12,DTAB*13,DTAB*14,DTAB*15,0};
int line[LNSIZE] = {0};
int word[WDSIZE] = {0};
int blockxxx[EVS-68-NHYP-NTAB-WDSIZE-LNSIZE] = {0};
#endif
/*spare 5 words*/
int oline[LNSIZE+1];
